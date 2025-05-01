#include "globals.hpp"
#include "packets.hpp"
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


void  serverAsReader(int sockfd, Context* config) {

    struct sockaddr_in recvAddr;
    socklen_t recvAddrLen = sizeof(recvAddr);
    struct sockaddr_in clientAddr{};
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(config->clientPort);
    inet_pton(AF_INET, config->clientIp, &clientAddr.sin_addr);

    //init var [intially config->curr_Win = config->WindowSize; ]
    unsigned char buffer[BUFFER_SIZE];
    int last_sent_ack = -1;
    fd_set readfds; // for async Read[Non Block IO]
    struct timeval timeout;
    //for not sending too many acks on recv wrong data pack
    std::chrono::_V2::steady_clock::time_point last_send_time = std::chrono::steady_clock::now();
    std::chrono::_V2::steady_clock::time_point current_time = std::chrono::steady_clock::now();

    // Send ACK-0 and Wait For Data Packet[blk =0] As Reply [Intially config->current_blk=0]
    unsigned char* ack_packet = build_ack_packet(config->current_blk);
    while (true) {
        //Send Read Request
        LOG("Send ACK- ",config->current_blk,"\n");
        sendto(sockfd, ack_packet, 4, 0, (struct sockaddr*)&clientAddr, sizeof(clientAddr));
        //setup for timeout
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        timeout.tv_sec = TIMEOUT_MILLI_SEC/1000;
        timeout.tv_usec = (TIMEOUT_MILLI_SEC % 1000)*1000;

        int activity = select(sockfd + 1, &readfds, nullptr, nullptr, &timeout);
        if(activity < 0){
            LOG("Select Sys Call Failed inside clientAsReader\n");
            exit(1);
        }
        else if (activity == 0) { // Timeout, resend RRQ
            LOG("Timeout! ReSend ACK- ",config->current_blk,"\n");
            continue;
        }
        else{
            break;
        }
    }// end of inf loop

    free(ack_packet);
    bool firstDataPkt = true; // Denotes 1st Packet[Data] recv after send Ack-0

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
                exit(1);
            }
            else if (activity == 0) { // Timeout:- Waited for Data Pkt bt Not recv
                LOG("Timeout For Expected Blk Num ",config->current_blk,"\n");
                unsigned char* ack_packet = build_ack_packet(config->current_blk);
                sendto(sockfd, ack_packet, 4, 0, (struct sockaddr*)&clientAddr, sizeof(clientAddr));
                free(ack_packet);
                config->curr_Win = config->WindowSize;
                continue;
            }
        }

        bzero(buffer,BUFFER_SIZE);
        ssize_t recv_len = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&recvAddr, &recvAddrLen);
        firstDataPkt = false;

        if (buffer[1] == 3) { // Data packet
            DATA_Packet data_pkt = extract_data_packet(buffer, recv_len);

            LOG("Expected Blk Num ",config->current_blk," Recv Blk Num ",data_pkt.block_number,"\n");

            if (data_pkt.block_number == config->current_blk) {
                writeFileBlock(config->fileName,data_pkt.data, data_pkt.data_size);
                config->current_blk++;
                config->curr_Win--;
            }
            else{//data_pkt.block_number > config->current_blk  or data_pkt.block_number < config->current_blk
                //Send the Acks only after certain intervals [TIMEOUT_MILLI_SEC]
                //prevents sending too many same ACKS
                current_time = std::chrono::steady_clock::now();
                if((last_sent_ack!= config->current_blk) or std::chrono::duration_cast<std::chrono::milliseconds>(current_time - last_send_time).count()>=TIMEOUT_MILLI_SEC){
                    LOG("Sending ACK with blk = ",config->current_blk,"\n");
                    unsigned char* ack_packet = build_ack_packet(config->current_blk);
                    sendto(sockfd, ack_packet, 4, 0, (struct sockaddr*)&clientAddr, sizeof(clientAddr));
                    free(ack_packet);
                    config->curr_Win = config->WindowSize;
                    last_send_time = current_time;
                    last_sent_ack = config->current_blk;
                }
                else{
                    LOG("Just Sent a ACK few ms ago \n");
                }
                free(data_pkt.data);
                continue;
            }

            if((config->curr_Win == 0) and !(data_pkt.data_size < 512)){
                LOG("Full Win Recv :- Sending ACK with blk = ",config->current_blk,"\n");
                unsigned char* ack_packet = build_ack_packet(config->current_blk);
                sendto(sockfd, ack_packet, 4, 0, (struct sockaddr*)&clientAddr, sizeof(clientAddr));
                free(ack_packet);
                config->curr_Win = config->WindowSize;
                free(data_pkt.data);
                continue;
            }

            free(data_pkt.data);//free resource
            if(data_pkt.data_size < 512) break; // Last data packet
        }

    }// end of inf loop

    //Handle Last Packet Recv:-
     while (true) {
        //Send Final ACK
        LOG("Sending Final ACK with blk = ",config->current_blk,"\n");
        unsigned char* ack_packet = build_ack_packet(config->current_blk);
        sendto(sockfd, ack_packet, 4, 0, (struct sockaddr*)&clientAddr, sizeof(clientAddr));
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
    // reach here means full file transfer complete
}
