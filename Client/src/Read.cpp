#include "c_globals.hpp"
#include "c_packets.hpp"

void writeFileBlock(const std::string& fileName,std::string data , int data_size){
    const char* fileMode = "ab";  // "ab" for append binary, "a" for append text

    // Open the file in the specified mode
    FILE* file = fopen(fileName.c_str(), fileMode);
    if (!file) {
        std::cerr << "Error opening file: " << fileName << std::endl;
        return;
    }

    //writing to file
    fwrite(data.c_str(), sizeof(unsigned char), data_size, file);

    // Close the file
    fclose(file); 

}

#define TIMEOUT_SEC 2

void clientAsReader(Config& config) {

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
    int curr_Win = config.serverWindowSize, expect_blk_num = 0;
    unsigned char buffer[BUFFER_SIZE];
    fd_set readfds; // for async Read[Non Block IO]
    struct timeval timeout;

    // Send RD packet and Wait For Data Packet[blk =1] As Reply
    unsigned char* rrq_packet = build_rrq_wrq_packet(config.filePath.c_str(), config.serverWindowSize, 1);
    while (true) {
        //Send Read Request
        std::cout<<"Send RD packet\n";        
        sendto(sockfd, rrq_packet, strlen(config.filePath.c_str()) + 4, 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
        //setup for timeout
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        timeout.tv_sec = TIMEOUT_SEC;
        timeout.tv_usec = 0;
        
        int activity = select(sockfd + 1, &readfds, nullptr, nullptr, &timeout);
        if(activity < 0){
            std::cout<<"Select Sys Call Failed inside clientAsReader\n";
            close(sockfd);
            exit(1);
        }
        else if (activity == 0) { // Timeout, resend RRQ
            std::cout << "Timeout! Resending Read Request." << std::endl;
            continue;
        }
        else{
            break;
        }
    }// end of inf loop
    
    free(rrq_packet);
    bool flag = true; // Denotes 1st Packet recv after send RD packet

    // Handle when Recv Data Packets
    while (true) {

        if(flag == false){ // check For Timeouts of recv Data Packets
            FD_ZERO(&readfds);
            FD_SET(sockfd, &readfds);
            timeout.tv_sec = TIMEOUT_SEC;
            timeout.tv_usec = 0;
            
            int activity = select(sockfd + 1, &readfds, nullptr, nullptr, &timeout);
            if(activity < 0){
                std::cout<<"Select Sys Call Failed inside clientAsReader\n";
                close(sockfd);
                exit(1);
            }
            else if (activity == 0) { // Timeout:- Waited for Data Pkt bt Not recv
                std::cout<<"Timeout For Expected Blk Num "<<expect_blk_num<<"\n";
                unsigned char* ack_packet = build_ack_packet(expect_blk_num);
                sendto(sockfd, ack_packet, 4, 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
                free(ack_packet);
                curr_Win = config.serverWindowSize;
                continue;
            }         
        }

        bzero(buffer,BUFFER_SIZE);
        ssize_t recv_len = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&recvAddr, &recvAddrLen);
        flag = false;
        
        if (buffer[1] == 3) { // Data packet
            DATA_Packet data_pkt = extract_data_packet(buffer, recv_len);
            
            std::cout<<"Expected Blk Num "<<expect_blk_num<<" Recv Blk Num "<<data_pkt.block_number<<"\n";

            if (data_pkt.block_number == expect_blk_num) {
                writeFileBlock(config.filePath,data_pkt.data, data_pkt.data_size);
                expect_blk_num++;
                curr_Win--;
            }
            
            if(curr_Win == 0 or data_pkt.block_number > expect_blk_num  or data_pkt.block_number< expect_blk_num){
                unsigned char* ack_packet = build_ack_packet(expect_blk_num);
                sendto(sockfd, ack_packet, 4, 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
                free(ack_packet);
                curr_Win = config.serverWindowSize;
            }

            if(data_pkt.data_size < 512) break; // Last data packet
        }
                
    }// end of inf loop

    std::cout<< "Received Full File \n";  
    close(sockfd);
}




