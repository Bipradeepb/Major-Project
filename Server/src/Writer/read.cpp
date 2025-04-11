#include "globals.hpp"
#include "packets.hpp"
#include "Logger.hpp"

void readThread(int sockfd, Context* ctx) {
    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);
    fd_set read_fds;
    struct timeval timeout;
    int last_ack_blk = ctx->current_blk;

    while (true) {
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);

        timeout.tv_sec = TIMEOUT_SEC;
        timeout.tv_usec = 0;

        int activity = select(sockfd + 1, &read_fds, NULL, NULL, &timeout);

        // ACK packet recv
        if (activity > 0) {
            unsigned char ack_buf[4];
            int recv_len = recvfrom(sockfd, ack_buf, sizeof(ack_buf), 0, (struct sockaddr*)&clientAddr, &addrLen);

            if (recv_len > 0) {
                ACK_Packet ack = extract_ack_packet(ack_buf);
                LOG("ACK recv with blk num = ",ack.block_number,"\n");
                std::lock_guard<std::mutex> lock(mtx);
                ctx->current_blk = ack.block_number; // Update current block
                last_ack_blk = ctx->current_blk;
                flag_Ack_Recv= true; // technically means current_blk updated
            }
        }
        // Timeout case for ACK
        else if (activity == 0) {
            LOG("Timeout For ACK\n");
            std::lock_guard<std::mutex> lock(mtx);
            ctx->current_blk = last_ack_blk; // Retransmit from last ACKed block
            flag_Ack_Recv= true;
        }

        //Signal Recv from Frwd thread to stop
        if(stop_flag == true)
            break;
    }// end of inf loop
}
