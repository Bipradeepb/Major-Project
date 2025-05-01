#include "c_globals.hpp"
#include "c_packets.hpp"
#include "Logger.hpp"

void readThread(int sockfd, const Config* ctx);
void forwardThread(int sockfd, const Config* ctx);

void clientAsWriter(const Config& config){

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
    unsigned char buffer[BUFFER_SIZE];
    fd_set readfds; // for async Read[Non Block IO]
    struct timeval timeout;

    //Build And Sent WR and wait For ACK 0 as Reply
    unsigned char* rrq_packet = build_rrq_wrq_packet(config.filePath.c_str(), config.serverWindowSize, 2);
    while (true) {
        //Send Read Request
        LOG_TO(LogDestination::TERMINAL_ONLY,"Send WR packet\n");
        sendto(sockfd, rrq_packet, strlen(config.filePath.c_str()) + 5, 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
        //setup for timeout
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        timeout.tv_sec = TIMEOUT_MILLI_SEC/1000;
        timeout.tv_usec = (TIMEOUT_MILLI_SEC % 1000)*1000;

        int activity = select(sockfd + 1, &readfds, nullptr, nullptr, &timeout);
        if(activity < 0){
            LOG("Select Sys Call Failed inside clientAsWriter\n");
            close(sockfd);
            exit(1);
        }
        else if (activity == 0) { // Timeout, resend WRQ
            LOG("Timeout! Resending Write Request.\n");
            continue;
        }
        else{
            break;
        }
    }// end of inf loop

    //Read ACK -0
    free(rrq_packet);
    bzero(buffer,BUFFER_SIZE);
    recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&recvAddr, &recvAddrLen);
    LOG("Receveied ACK - 0\n");


    std::thread readThreadInstance(readThread, sockfd, &config);
    std::thread forwardThreadInstance(forwardThread, sockfd, &config);

    readThreadInstance.join();
    forwardThreadInstance.join();

    LOG_TO(LogDestination::TERMINAL_ONLY,"File Transfer Complete \n");
    LOG_TO(LogDestination::FILE_ONLY,"File Transfer Complete \n");
    close(sockfd);

}
