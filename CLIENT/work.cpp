#include "headers/globals.hpp"
#include "headers/handle_packets.cpp"
#include "headers/helper_functions.cpp"

using namespace std;

#define aliveCount 9000
std::chrono::steady_clock::time_point setTime;

void handleNackJob(int req_nack, int sockfd)
{
    Packet* packet_to_retransmit = nullptr;
    bool found = false;

    mtx_window.lock();
    for (Packet* pkt : window)
    {
        if (pkt->ackno == req_nack)
        {
            packet_to_retransmit = pkt; // Create a copy
            found = true;
            break;
        }
    }
    mtx_window.unlock();

    if (found && packet_to_retransmit)
    {
        mtx_server_addr.lock();
        ssize_t sent_bytes = sendto(sockfd, packet_to_retransmit->data, BUFFER_SIZE, 0,
                                    (const struct sockaddr*)&server_addr, sizeof(server_addr));
        mtx_server_addr.unlock();

        if (sent_bytes == -1)
        {
            perror("sendto failed");
        }
    }

    packet_to_retransmit=nullptr;
}

void worker_thread(int sockfd)
{
    cout<<"Started Worker Thread..."<<"\n";
    char buffer[BUFFER_SIZE];
    memset(buffer,'\0',BUFFER_SIZE);

    while(1)
    {
        //grab a lock
        std::unique_lock<std::mutex> lock(mtx_jobQueue);

        if (mode == 1 && !isTransmissionON) 
        {
            // Wait only for aliveCount duration
            auto timeoutResult = cv_jobQueue.wait_for(lock, std::chrono::milliseconds(aliveCount), [] {
                return !jobQueue.empty();
            });

            if (!timeoutResult) {
                // Timeout occurred, no job received
                lock.unlock();
                return;
            }

            // A job arrived within aliveCount — process it below
        } 
        else {
            // Normal wait (no timeout)
            cv_jobQueue.wait(lock, [] {return !jobQueue.empty();});
        }

        //otherwise pop the first job 
        Job *job=jobQueue.front();
        jobQueue.pop();
        lock.unlock(); //release the queue 

        //process the obtained job
        switch(job->jobId){

            case 1:{
                //this is the read job
                //no use of read job in case of client
                break;
            }

            case 2:{

                //write job 
                //no use of write job in case of client
                break;
            }

            case 3:{

                //this is an ACK Job

                //extract the ACK from job
                int next_ack;
                std::memcpy(&next_ack,job->data,job->dataSize);

                //check if the ACK is valid
                if(next_ack>=startSEQN&&next_ack<=finalSEQN)
                {
                    //this ACK is old
                    //reject this ack;
                    break;
                }

                if(!isTransmissionON)
                {
                    //this is the final ACK
                    //in case of client we need to end the worker thread
                    //since transmission ended and last ACK has been received
                    // resetDS();
                    cout<<"Final Ack received.\n";
                    return;
                }

                //update the window sequence numbers
                startSEQN=(startSEQN+windowSize)%SEQN;
                finalSEQN=(finalSEQN+windowSize)%SEQN;
                cur_ack_no=startSEQN;

                //reset the current window
                resetWindow();

                mtx_window.lock();
                {
                   //form the new window of packets
                    for(int i=1;i<=windowSize;i++)
                    {
                        auto pr=build_data_packet();
                        size_t dataSize=pr.second;
                        Packet *packet=pr.first;

                        window.push_back(packet);

                        //check if last block has been read
                        if(cur_block_ptr>=getFileSize())
                        {
                            //if the data in the packet is less than 512 
                            if(dataSize<512)
                            {
                                isTransmissionON=false;
                                break;
                            }

                            //if the data in the packet is equal to 512
                            if(dataSize==512&&i==windowSize)
                            {
                                break;
                            }
                            //if it is the last packet of this window,keep transmission ON
                            //transmit a NULL Packet 
                            isTransmissionON=false;
                            auto pr1=build_data_packet();
                            Packet *nullPacket=pr1.first;
                            window.push_back(nullPacket);

                            break;
                        }
                        //update cur_ack_no
                        cur_ack_no++;
                    }
                    //reset cur_ack_no
                    cur_ack_no=startSEQN; 
                }

                mtx_window.unlock();

                {
                    std::unique_lock<std::mutex> lock(mtx);

                    if(should_sleep)
                    {
                        should_sleep=false;
                        cv.notify_all();
                    }
                    else
                    {
                        cv.wait(lock,[]{return should_sleep;});
                        should_sleep=false;
                        cv.notify_all();
                    }
                }
                break;
            }

            case 4:{

                //this is a NACK job

                int req_nack;
                std::memcpy(&req_nack,job->data,job->dataSize);

                if(req_nack>=startSEQN&&req_nack<=finalSEQN)
                {
                    //search in the window of packets and only send that singular packet
                    handleNackJob(req_nack,sockfd);
                }

                break;
            }

            case 5:{

                //this is job for data packet

                //extract the ack 
                //extract the actual packet data from the job buffer
                int ackno;
                int bytesToWrite;
                memset(buffer,'\0',BUFFER_SIZE);
                std::memcpy(&ackno,job->data,sizeof(int));
                std::memcpy(&bytesToWrite,job->data+4,sizeof(int));
                std::memcpy(buffer,job->data+8,bytesToWrite);

                Packet* packet=new Packet(ackno,buffer,bytesToWrite);

                bool inWindow=(ackno>=startSEQN&&ackno<=finalSEQN);

                //check if the data has been retransmitted by the sender
                //ack was lost, retransmit previous ack
                if(!inWindow)
                {
                    Packet *ACKpacket=build_ack_packet(startSEQN);
                    transmitACK(ACKpacket,sockfd);
                    break;
                }

                //if data belongs to current window insert into receiver window
                {
                    mtx_receiverWindow.lock();
                    auto result=receiverWindow.insert(packet);
                    if(!result.second)
                    {
                        delete packet;
                    }

                    //if the receiver window size is full
                    if(receiverWindow.size()==windowSize)
                    {
                        //update seqn numbers
                        startSEQN=(startSEQN+windowSize)%SEQN;
                        finalSEQN=(finalSEQN+windowSize)%SEQN;

                        //->transmit an ACk
                        Packet *ACKpacket=build_ack_packet(startSEQN);
                        transmitACK(ACKpacket,sockfd);

                        //->write all the packets to file using helper function
                        //receiver window must be cleared

                        for(Packet *dataPacket:receiverWindow)
                        {
                            cout<<"Writing data packet with ackno="<<dataPacket->ackno<<"\n";
                            ssize_t bytesWritten=writeToFile(dataPacket->data,dataPacket->dataSize);
                        }
                        //clear the window(transmission) as well
                        resetReceiverWindow();
                    }
                    mtx_receiverWindow.unlock();
                }
                //if the receiver window size is not yet full
                //the sender will manage the NACKs.
                break;
            }

            case 6:{

                //this job is to end transmission
                //this represents final data packet

                //extract the ack 
                //extract the actual packet data from the job buffer
                int ackno;
                int bytesToWrite;
                memset(buffer,'\0',BUFFER_SIZE);
                std::memcpy(&ackno,job->data,sizeof(int));
                std::memcpy(&bytesToWrite,job->data+4,sizeof(int));
                std::memcpy(buffer,job->data+8,bytesToWrite);

                Packet *packet=new Packet(ackno,buffer,bytesToWrite);

                bool inWindow=(ackno>=startSEQN&&ackno<=finalSEQN);

                //check if the data has been retransmitted by the sender
                //ack was lost, retransmit previous ack
                if(!inWindow)
                {
                    Packet *ACKpacket=build_ack_packet(startSEQN);
                    transmitACK(ACKpacket,sockfd);
                    //also handles the case if the last ACK is lost
                    break;
                }

                //this is the last data packet  of transmission in current Window
                //insert it into receiver window
                {
                    mtx_receiverWindow.lock();

                    receiverWindow.insert(packet);
                    //write to File all the packets received
                    for(Packet *dataPacket:receiverWindow)
                    {
                        cout<<"Writing data packet with ackno="<<dataPacket->ackno<<"\n";
                        ssize_t bytesWritten=writeToFile(dataPacket->data,dataPacket->dataSize);
                    }

                    resetReceiverWindow();

                    mtx_receiverWindow.unlock();
                }

                //update seqn numbers
                startSEQN=(startSEQN+windowSize)%SEQN;
                finalSEQN=(finalSEQN+windowSize)%SEQN;

                //send final ACK
                Packet *ACKpacket=build_ack_packet(startSEQN);
                transmitACK(ACKpacket,sockfd);

                cout<<"Final ACK transmitted.\n";
                
                //also should terminate the client but after a threshold period
                setTime = std::chrono::steady_clock::now();  // Store time of last packet
                isTransmissionON = false;  // Set transmission flag to false (end transmission)

                break;
            }

            case 7:{

                //this job is to handle error
                cout<<"Error has occurred at the client side.Ending transmission..."<<"\n";
                // resetDS();
                //need to terminate the transmission 
                return;
            }

            default:{

                cout<<"Wrong JobId. Skipping Job"<<endl;
                break;
            }
        }

        delete job;
        job=nullptr;
    }
}