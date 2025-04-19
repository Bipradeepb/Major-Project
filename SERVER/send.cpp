#include "headers/globals.hpp"
#include "headers/helper_functions.cpp"
#include "headers/handle_packets.cpp"

#define TIMEOUT 8000
using namespace std;

void sender_thread(int sockfd,Context *ctx)
{
    cout<<"Started Sender Thread..."<<"\n";
    
    while(true)
    {
        //sync with client
        struct sockaddr_in clientaddr{};
        clientaddr.sin_family = AF_INET;
        clientaddr.sin_port = htons(ctx->clientPort);
        inet_pton(AF_INET, ctx->clientIp, &clientaddr.sin_addr);

        //continuously process and send packets
        if(ctx->mode==0)
        {
            //server is running in sender mode
            //transmit all the packets in the transmission window
            mtx_window.lock();

            for(auto &packet:ctx->window)
            {
                ssize_t sent_bytes = sendto(sockfd, packet.data, BUFFER_SIZE, 0,
                                            (const struct sockaddr*)&clientaddr, sizeof(clientaddr));

                if (sent_bytes == -1)
                {
                    perror("sendto failed");
                    exit(1);
                }
            }

            if(!ctx->window.empty()){
                cout<<"Window transmitted to client.\n";
                LOG("Window transmitted to client.","");
            }
            
            mtx_window.unlock();
        }
        else
        {
            //server is in receiver mode
            //traverse through the receiverWindow and send NACK packet
            //for packets still not yet received
            //do this only if receiver window >=50% full
            //this is to prevent sending excessive NACKS

            std::vector<int> NACKS;
            
            {
                mtx_receiverWindow.lock();
                int receiverWindowSize=ctx->receiverWindow.size();

                if(receiverWindowSize>=ctx->windowSize/2)
                {
                    for(int seqno=ctx->startSEQN;seqno<=ctx->finalSEQN;seqno++)
                    {
                        Packet dummy(seqno,"",0);

                        auto it = ctx->receiverWindow.lower_bound(dummy);
                        bool found = (it != ctx->receiverWindow.end() && (*it).ackno == seqno);

                        if(!found)
                        {
                            NACKS.push_back(seqno);
                        }
                    }
                }

                mtx_receiverWindow.unlock();
            }

            //transmit the nacks now

            for(int ackno:NACKS)
            {
                Packet packet=build_nack_packet(ackno,ctx);

                ssize_t sent_bytes = sendto(sockfd, packet.data, BUFFER_SIZE, 0,
                                            (const struct sockaddr*)&clientaddr, sizeof(clientaddr));

                cout<<"Transmitting NACK="<<ackno<<"\n";
                LOG("Transmitting NACK",to_string(ackno));

                if (sent_bytes == -1)
                {
                    perror("sendto failed");
                    exit(1);
                }
            }
            //transmitted the NACKS
        }

        {
            // Wait with timeout, but allow interruption
            std::unique_lock<std::mutex> lock(mtx);
            // Mark the thread as asleep
            ctx->should_sleep = true;
            cv.notify_one();
            cv.wait_for(lock, std::chrono::milliseconds(TIMEOUT), [ctx] { return !ctx->should_sleep; });
            ctx->should_sleep = false;
        }
    }
}