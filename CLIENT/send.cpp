#include "headers/globals.hpp"
#include "headers/helper_functions.cpp"
#include "headers/handle_packets.cpp"

#define TIMEOUT 8000
using namespace std;

void sender_thread(int sockfd)
{
    cout<<"Started Sender Thread..."<<"\n";

    while(!stop_thread)
    {
        //continuously process and send packets
        if(mode==0)
        {
            //server is running in sender mode
            //transmit all the packets in the transmission window

            std::unique_lock<std::mutex> lock(mtx_window);

            for(Packet *packet:window)
            {
                mtx_server_addr.lock();
                ssize_t sent_bytes = sendto(sockfd, packet->data, BUFFER_SIZE, 0,
                                            (const struct sockaddr*)&server_addr, sizeof(server_addr));
                mtx_server_addr.unlock();
                if (sent_bytes == -1)
                {
                    perror("sendto failed");
                    exit(1);
                }
            }

            lock.unlock();
        }
        else
        {
            //server is in receiver mode
            //traverse through the receiverWindow and send NACK packet
            //for packets still not yet received
            //do this only if receiver window >=50% full
            //this is to prevent sending excessive NACKS

            vector<int> NACKS;
            std::unique_lock<std::mutex> lock(mtx_receiverWindow);

            int receiverWindowSize=receiverWindow.size();

            if(receiverWindowSize>=windowSize/2)
            {
                for(int seqno=startSEQN;seqno<=finalSEQN;seqno++)
                {
                    Packet dummy(seqno,"",0);

                    auto it = receiverWindow.lower_bound(&dummy);
                    bool found = (it != receiverWindow.end() && (*it)->ackno == seqno);

                    if(!found)
                    {
                        NACKS.push_back(seqno);
                    }
                }
                lock.unlock();
            }
            else
            {
                lock.unlock();
                //transmit an ACK with starting seqno number
                Packet *packet=build_ack_packet(startSEQN);
                transmitACK(packet,sockfd);
            }

            //transmit the nacks now
            for(int ackno:NACKS)
            {
                Packet *packet=build_nack_packet(ackno);

                mtx_server_addr.lock();
                ssize_t sent_bytes = sendto(sockfd, packet->data, BUFFER_SIZE, 0,
                                            (const struct sockaddr*)&server_addr, sizeof(server_addr));
                mtx_server_addr.unlock();

                if (sent_bytes == -1)
                {
                    perror("sendto failed");
                    exit(1);
                }

                delete packet;
                packet=nullptr;
            }
            //transmitted the NACKS
        }

        {
            // Wait with timeout, but allow interruption
            std::unique_lock<std::mutex> lock(mtx);
            // Mark the thread as asleep
            should_sleep = true;
            cv.notify_one();
            cv.wait_for(lock, std::chrono::milliseconds(TIMEOUT), [] { return !should_sleep; });
            should_sleep = false;
        }
    }
}