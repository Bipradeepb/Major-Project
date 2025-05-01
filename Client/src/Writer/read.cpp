#include "c_globals.hpp"
#include "c_packets.hpp"
#include "Logger.hpp"

void readThread(int sockfd, const Config* ctx) {
    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);
    fd_set read_fds;
    struct timeval timeout;
    int last_ack_blk = current_blk;

    while (true) {
        //resetting timer
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);
        timeout.tv_sec = TIMEOUT_MILLI_SEC/1000;
        timeout.tv_usec = (TIMEOUT_MILLI_SEC % 1000)*1000;

        //LOG("Before select\n");
        int activity = select(sockfd + 1, &read_fds, NULL, NULL, &timeout);
        //LOG("After select\n");

        // ACK packet recv
        if (activity > 0) {
            //LOG("Before recvfrom\n");
            unsigned char ack_buf[4];
            int recv_len = recvfrom(sockfd, ack_buf, sizeof(ack_buf), 0, (struct sockaddr*)&clientAddr, &addrLen);
            //LOG("After Recv from\n");
            if (recv_len > 0) {
                ACK_Packet ack = extract_ack_packet(ack_buf);
                LOG("ACK recv with blk num = ",ack.block_number,"\n");
                std::lock_guard<std::mutex> lock(mtx);
                //LOG("Updating inside recv ACK blk\n")
                current_blk = ack.block_number; // Update current block
                last_ack_blk = current_blk;
                flag_Ack_Recv= true; // signals frwd thread to send next Window
                if(ack.block_number > last_D_blk){
                    read_thread_end =true;
                    //LOG("Ending Read thread\n");
                    break; // end read thread
                }
                //LOG("Update over -recv ack blk\n");
            }
        }
        // Timeout case for ACK
        else if (activity == 0) {
            LOG("Timeout For ACK\n");
            std::lock_guard<std::mutex> lock(mtx);
            current_blk = last_ack_blk; // Retransmit from last ACKed block
            flag_Ack_Recv= true;
            //LOG("Update over -timeout ack blk\n");
        }

        //LOG("Continue while true loop\n");

    }//inf while loop ends
}//end of read thread
