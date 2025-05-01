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
        //reset timer
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
                std::lock_guard<std::mutex> lock(mtx);
                ctx->current_blk = ack.block_number; // Update current block
                last_ack_blk = ctx->current_blk;
                flag_Ack_Recv= true; // signals frwd thread to send next Window
                if(ack.block_number > last_D_blk){
                    read_thread_end =true;
                    break; // end read thread
                }
            }
        }
        // Timeout case for ACK
        else if (activity == 0) {
            LOG("Timeout For ACK\n");
            std::lock_guard<std::mutex> lock(mtx);
            ctx->current_blk = last_ack_blk; // Retransmit from last ACKed block
            flag_Ack_Recv= true;
        }

    }// end of inf loop
}
