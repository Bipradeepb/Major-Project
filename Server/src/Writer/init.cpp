#include "globals.hpp"

void readThread(int sockfd, Context* ctx);
void forwardThread(int sockfd, Context* ctx);

void serverAsWriter(int sockfd, Context* ctx) {
    std::thread readThreadInstance(readThread, sockfd, ctx );
    std::thread forwardThreadInstance(forwardThread, sockfd, ctx);

    readThreadInstance.join();
    forwardThreadInstance.join();
}
