#include "globals.hpp"

void readThread(int sockfd, Context* ctx,bool flag_Ack_Recv);
void forwardThread(int sockfd, Context* ctx,bool flag_Ack_Recv);

void serverAsWriter(int sockfd, Context* ctx) {
    bool flag_Ack_Recv = false;
    std::thread readThreadInstance(readThread, sockfd, ctx , flag_Ack_Recv);
    std::thread forwardThreadInstance(forwardThread, sockfd, ctx, flag_Ack_Recv);

    readThreadInstance.join();
    forwardThreadInstance.join();
}
