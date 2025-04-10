#include "globals.hpp"
#include "packets.hpp"

void writeFileBlock(const std::string& fileName,const std::string& data , int data_size){
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


void  serverAsReader(int sockfd, Context* config) {

    struct sockaddr_in recvAddr;
    socklen_t recvAddrLen = sizeof(recvAddr);
    struct sockaddr_in clientAddr{};
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(config->clientPort);
    inet_pton(AF_INET, config->clientIp, &clientAddr.sin_addr);

    //init var [intially config->curr_Win = config->WindowSize; ]
    unsigned char buffer[BUFFER_SIZE];
    fd_set readfds; // for async Read[Non Block IO]
    struct timeval timeout;

    // Send ACK-0 and Wait For Data Packet[blk =0] As Reply [Intially config->current_blk=0]
    unsigned char* ack_packet = build_ack_packet(config->current_blk);
    while (true) {
        //Send Read Request
        std::cout<<"Send ACK- "<<config->current_blk<<"\n";
        sendto(sockfd, ack_packet, 4, 0, (struct sockaddr*)&clientAddr, sizeof(clientAddr));
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
            std::cout<<"Timeout! ReSend ACK- "<<config->current_blk<<"\n";
            continue;
        }
        else{
            break;
        }
    }// end of inf loop

    free(ack_packet);
    bool flag = true; // Denotes 1st Packet[Data] recv after send Ack-0

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
                std::cout<<"Timeout For Expected Blk Num "<<config->current_blk<<"\n";
                unsigned char* ack_packet = build_ack_packet(config->current_blk);
                sendto(sockfd, ack_packet, 4, 0, (struct sockaddr*)&clientAddr, sizeof(clientAddr));
                free(ack_packet);
                config->curr_Win = config->WindowSize;
                continue;
            }
        }

        bzero(buffer,BUFFER_SIZE);
        ssize_t recv_len = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&recvAddr, &recvAddrLen);
        flag = false;

        if (buffer[1] == 3) { // Data packet
            DATA_Packet data_pkt = extract_data_packet(buffer, recv_len);

            std::cout<<"Expected Blk Num "<<config->current_blk<<" Recv Blk Num "<<data_pkt.block_number<<"\n";

            if (data_pkt.block_number == config->current_blk) {
                writeFileBlock(config->fileName,data_pkt.data, data_pkt.data_size);
                config->current_blk++;
                config->curr_Win--;
            }
            else{//data_pkt.block_number > config->current_blk  or data_pkt.block_number < config->current_blk
                std::cout<<"Sending ACK with blk = "<<config->current_blk<<"\n";
                unsigned char* ack_packet = build_ack_packet(config->current_blk);
                sendto(sockfd, ack_packet, 4, 0, (struct sockaddr*)&clientAddr, sizeof(clientAddr));
                free(ack_packet);
                config->curr_Win = config->WindowSize;
            }
            //std::cout<<"config->curr_Win = "<<config->curr_Win<<"\n";
            if(config->curr_Win == 0){
                std::cout<<"Full Win Recv :- Sending ACK with blk = "<<config->current_blk<<"\n";
                unsigned char* ack_packet = build_ack_packet(config->current_blk);
                sendto(sockfd, ack_packet, 4, 0, (struct sockaddr*)&clientAddr, sizeof(clientAddr));
                free(ack_packet);
                config->curr_Win = config->WindowSize;
            }

            free(data_pkt.data);//free resource
            if(data_pkt.data_size < 512) break; // Last data packet
        }

    }// end of inf loop

    //Handle Last Packet Recv:-
     while (true) {
        //Send Final ACK
        std::cout<<"Sending FInal ACK with blk = "<<config->current_blk<<"\n";
        unsigned char* ack_packet = build_ack_packet(config->current_blk);
        sendto(sockfd, ack_packet, 4, 0, (struct sockaddr*)&clientAddr, sizeof(clientAddr));
        free(ack_packet);

        //setup for timeout
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        timeout.tv_sec = TIMEOUT_SEC;
        timeout.tv_usec = 0;

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

    std::cout<< "Received Full File \n";
    close(sockfd);


}
