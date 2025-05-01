#include "globals.hpp"
#include "packets.hpp"
#include "Logger.hpp"

std::pair<unsigned char*,size_t> readFileBlock(const std::string& fileName, int block_num, const std::string& mode);

void readThread(int sockfd, Context* ctx) {
    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);
    fd_set read_fds;
    struct timeval timeout;
    int last_ack_blk = ctx->current_blk;
    stop_flag = false;

    while (true) {
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);

        timeout.tv_sec = TIMEOUT_MILLI_SEC/1000;
        timeout.tv_usec = (TIMEOUT_MILLI_SEC % 1000)*1000;

        int activity = select(sockfd + 1, &read_fds, NULL, NULL, &timeout);

        // ACK packet recv
        if (activity > 0) {
            unsigned char ack_buf[4];
            int recv_len = recvfrom(sockfd, ack_buf, sizeof(ack_buf), 0, (struct sockaddr*)&clientAddr, &addrLen);

            if (recv_len > 0) {
                ACK_Packet ack = extract_ack_packet(ack_buf);
                LOG("ACK recv with blk num = ",ack.block_number,"\n");
                //Signal Recv from Frwd thread to stop => this Ack recv is last ACK
                if(stop_flag == true)
                    break;
                std::lock_guard<std::mutex> lock(mtx);
                ctx->current_blk = ack.block_number; // Update current block
                last_ack_blk = ctx->current_blk;
                flag_Ack_Recv= true; // technically means current_blk updated
            }
        }
        // Timeout case for ACK
        else if (activity == 0) {
            LOG("Timeout For ACK\n");

            //sent last blk but not recv ACK ie Resend Last Blk and wait for ACK
            if(stop_flag == true){
                auto blk = readFileBlock(ctx->fileName, ctx->current_blk -1, "octet");
                LOG("Building Data Packet for blk = ",ctx->current_blk-1,"\n");
                u_char * packet = build_data_packet(ctx->current_blk-1,blk.first,blk.second);
                struct sockaddr_in clientAddr;
                clientAddr.sin_family = AF_INET;
                clientAddr.sin_port = htons(ctx->clientPort);
                inet_pton(AF_INET, ctx->clientIp, &clientAddr.sin_addr);
                sendto(sockfd, packet, blk.second + 4, 0, (struct sockaddr*)&clientAddr, sizeof(clientAddr));
                //free resource
                free(packet);
                delete [] (blk.first);
                continue;
            }

            std::lock_guard<std::mutex> lock(mtx);
            ctx->current_blk = last_ack_blk; // Retransmit from last ACKed block
            flag_Ack_Recv= true;
        }

    }// end of inf loop
}
