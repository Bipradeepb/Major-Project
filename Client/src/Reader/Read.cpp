#include "c_globals.hpp"
#include "c_packets.hpp"
#include "Logger.hpp"

void writeFileBlock(const std::string& fileName,const u_char* data , int data_size){
    const char* fileMode = "ab";  // "ab" for append binary, "a" for append text

    // Open the file in the specified mode
    FILE* file = fopen(fileName.c_str(), fileMode);
    if (!file) {
        std::cerr << "Error opening file: " << fileName << std::endl;
        return;
    }

    //writing to file
    fwrite(data, sizeof(unsigned char), data_size, file);

    // Close the file
    fclose(file);

}



void clientAsReader(const Config& config) {

    // Setup UDP Socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    check_err(sockfd, "Socket creation failed");
    struct sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(config.serverPort);
    inet_pton(AF_INET, config.serverIp.c_str(), &serverAddr.sin_addr);
    struct sockaddr_in recvAddr;
    socklen_t recvAddrLen = sizeof(recvAddr);

    //init var
    int curr_Win = config.serverWindowSize, expect_blk_num = 0 , last_sent_ack=-1;
    unsigned char buffer[BUFFER_SIZE];
    fd_set readfds; // for async Read[Non Block IO]
    struct timeval timeout;
    //for not sending too many acks on recv wrong data pack
    std::chrono::_V2::steady_clock::time_point last_send_time = std::chrono::steady_clock::now();
    std::chrono::_V2::steady_clock::time_point current_time = std::chrono::steady_clock::now();

    // Send RD packet and Wait For Data Packet[blk =0] As Reply
    unsigned char* rrq_packet = build_rrq_wrq_packet(config.filePath.c_str(), config.serverWindowSize, 1);
    while (true) {
        //Send Read Request
        LOG_TO(LogDestination::TERMINAL_ONLY,"Send RD packet\n");
        sendto(sockfd, rrq_packet, strlen(config.filePath.c_str()) + 5, 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
        //setup for timeout
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        timeout.tv_sec = TIMEOUT_MILLI_SEC/1000;
        timeout.tv_usec = (TIMEOUT_MILLI_SEC % 1000)*1000;

        int activity = select(sockfd + 1, &readfds, nullptr, nullptr, &timeout);
        if(activity < 0){
            LOG("Select Sys Call Failed inside clientAsReader\n");
            close(sockfd);
            exit(1);
        }
        else if (activity == 0) { // Timeout, resend RRQ
            LOG("Timeout! Resending Read Request.\n");
            continue;
        }
        else{
            break;
        }
    }// end of inf loop

    free(rrq_packet);
    bool firstDataPkt = true; // Denotes 1st Packet recv after send RD packet

    // Handle when Recv Data Packets
    while (true) {

        if(firstDataPkt == false){ // check For Timeouts of recv Data Packets
            FD_ZERO(&readfds);
            FD_SET(sockfd, &readfds);
            timeout.tv_sec = TIMEOUT_MILLI_SEC/1000;
            timeout.tv_usec = (TIMEOUT_MILLI_SEC % 1000)*1000;

            int activity = select(sockfd + 1, &readfds, nullptr, nullptr, &timeout);
            if(activity < 0){
                LOG("Select Sys Call Failed inside clientAsReader\n");
                close(sockfd);
                exit(1);
            }
            else if (activity == 0) { // Timeout:- Waited for Data Pkt bt Not recv
                LOG("Timeout For Expected Blk Num ",expect_blk_num,"\n");
                unsigned char* ack_packet = build_ack_packet(expect_blk_num);
                sendto(sockfd, ack_packet, 4, 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
                free(ack_packet);
                curr_Win = config.serverWindowSize;
                continue;
            }
        }

        bzero(buffer,BUFFER_SIZE);
        ssize_t recv_len = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&recvAddr, &recvAddrLen);
        firstDataPkt = false;

        if (buffer[1] == 3) { // Data packet
            DATA_Packet data_pkt = extract_data_packet(buffer, recv_len);

            LOG("Expected Blk Num ",expect_blk_num," Recv Blk Num ",data_pkt.block_number,"\n");

            if (data_pkt.block_number == expect_blk_num) {
                writeFileBlock(config.filePath,data_pkt.data, data_pkt.data_size);
                expect_blk_num++;
                curr_Win--;

                // //debug
                // LOG("Data packet recv (write to file)\n")
                // for (int i =0;i<data_pkt.data_size;++i) {
                //     std::cout<<(uint)data_pkt.data[i]<<" ";
                // }
                // LOG("\n");

            }
            else{//data_pkt.block_number > expect_blk_num  or data_pkt.block_number < expect_blk_num
                //prevents sending too many same ACKS
                current_time = std::chrono::steady_clock::now();
                if((last_sent_ack != expect_blk_num) or(std::chrono::duration_cast<std::chrono::milliseconds>(current_time - last_send_time).count()>=TIMEOUT_MILLI_SEC)){
                    LOG("Sending ACK with blk = ",expect_blk_num,"\n");
                    unsigned char* ack_packet = build_ack_packet(expect_blk_num);
                    sendto(sockfd, ack_packet, 4, 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
                    free(ack_packet);
                    curr_Win = config.serverWindowSize;
                    last_send_time = current_time;
                    last_sent_ack = expect_blk_num;
                }
                else{
                    LOG("Just Sent a ACK few ms ago \n");
                }
                free(data_pkt.data);
                continue;
            }

            if((curr_Win == 0) and !(data_pkt.data_size < 512)){
                LOG("Full Win Recv :- Sending ACK with blk = ",expect_blk_num,"\n");
                unsigned char* ack_packet = build_ack_packet(expect_blk_num);
                sendto(sockfd, ack_packet, 4, 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
                free(ack_packet);
                curr_Win = config.serverWindowSize;
                free(data_pkt.data);
                continue;
            }

            free(data_pkt.data);
            if(data_pkt.data_size < 512) break; // Last data packet
        }

    }// end of inf loop

    //Handle Last Packet Recv:-
     while (true) {
        //Send Final ACK
        LOG("Sending Final ACK with blk = ",expect_blk_num,"\n");
        unsigned char* ack_packet = build_ack_packet(expect_blk_num);
        sendto(sockfd, ack_packet, 4, 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
        free(ack_packet);

        //setup for timeout
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        timeout.tv_sec = TIMEOUT_MILLI_SEC/1000;
        timeout.tv_usec = (TIMEOUT_MILLI_SEC % 1000)*1000;

        int activity = select(sockfd + 1, &readfds, nullptr, nullptr, &timeout);

        if(activity == 0){// Timeout ==> Great We Can Finally END
            break;
        }
        else if(activity >0){ // Sent Final ACK lost ==> Server Resend Data Packet
            recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&recvAddr, &recvAddrLen);
            //discard this buffer
            continue;
        }
    }

    LOG_TO(LogDestination::TERMINAL_ONLY, "File Transfer Complete \n");
    LOG_TO(LogDestination::FILE_ONLY, "File Transfer Complete \n");
    close(sockfd);
}
