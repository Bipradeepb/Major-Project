#include "headers/globals.hpp"
#include "headers/handle_packets.cpp"
#include "headers/helper_functions.cpp"

using namespace std;

#define aliveCount 8000
std::chrono::steady_clock::time_point setTime;


void handleNackJob(int req_nack, int sockfd,Context *ctx)
{
    Packet* packet_to_retransmit = nullptr;
    bool found = false;

    mtx_window.lock();
    for (const auto& pkt : ctx->window)
    {
        if (pkt.ackno == req_nack)
        {
            packet_to_retransmit = new Packet(pkt.ackno, pkt.data, pkt.dataSize); // Create a copy
            found = true;
            break;
        }
    }
    mtx_window.unlock();

    struct sockaddr_in clientaddr{};
    clientaddr.sin_family = AF_INET;
    clientaddr.sin_port = htons(ctx->clientPort);
    inet_pton(AF_INET, ctx->clientIp, &clientaddr.sin_addr);

    if (found && packet_to_retransmit)
    {

        ssize_t sent_bytes = sendto(sockfd, packet_to_retransmit->data, BUFFER_SIZE, 0,
                                    (const struct sockaddr*)&clientaddr, sizeof(clientaddr));

        if (sent_bytes == -1)
        {
            perror("sendto failed");
        }

        delete packet_to_retransmit; // Free memory after use
    }
}

void transmitACK(Packet packet,int sockfd,Context *ctx)
{
    struct sockaddr_in clientaddr{};
    clientaddr.sin_family = AF_INET;
    clientaddr.sin_port = htons(ctx->clientPort);
    inet_pton(AF_INET, ctx->clientIp, &clientaddr.sin_addr);

    ssize_t sent_bytes = sendto(sockfd, packet.data, BUFFER_SIZE, 0,
                                (const struct sockaddr*)&clientaddr, sizeof(clientaddr));

    if (sent_bytes == -1)
    {
        perror("sendto failed");
        exit(1);
    }
}


void worker_thread(int sockfd,Context *ctx)
{
    cout<<"Started Worker Thread..."<<"\n";
    char buffer[BUFFER_SIZE];
    memset(buffer,'\0',BUFFER_SIZE);

    while(1)
    {
        //grab a lock
        std::unique_lock<std::mutex> lock(mtx_jobQueue);

        //new logic
        if (ctx->mode == 1 && !ctx->isTransmissionON) 
        {
            // Wait only for aliveCount duration
            auto timeoutResult = cv_jobQueue.wait_for(lock, std::chrono::milliseconds(aliveCount), [ctx] {
                return !ctx->jobQueue.empty();
            });

            if (!timeoutResult) {
                // Timeout occurred, no job received
                lock.unlock();
                resetDS(ctx);
                continue;
            }

            // A job arrived within aliveCount — process it below
        } 
        else {
            // Normal wait (no timeout)
            cv_jobQueue.wait(lock, [ctx] {return !ctx->jobQueue.empty();});
        }

        //otherwise pop the first job and process
        Job job=ctx->jobQueue.front();
        lock.unlock(); //release the queue 

        //process the obtained job
        switch(job.jobId){

            case 1:{
                //this is the read job

                //set mode
                ctx->mode=0;

                //extract filename from data
                //datasize->length of name of file
                std::memcpy(ctx->FILENAME,job.data,job.dataSize);
                ctx->FILENAME[job.dataSize]='\0';

                //reset cur_ack_no
                ctx->cur_ack_no=ctx->startSEQN;

                //reset block pointer
                ctx->cur_block_ptr=ctx->str_block_ptr;

                mtx_window.lock();
                {

                    //need to form window of windowsize packets
                    for(int i=1;i<=ctx->windowSize;i++)
                    {
                        auto pr=build_data_packet(ctx);
                        size_t dataSize=pr.second;
                        Packet packet=pr.first;

                        // ctx->window.push_back(packet);
                        ctx->window.insert(packet);

                        //check if last block has been read
                        if(ctx->cur_block_ptr>=getFileSize(ctx))
                        {
                            //if the data in the packet is less than 512 
                            if(dataSize<512)
                            {
                                ctx->isTransmissionON=false;
                                break;
                            }

                            //if the data in the packet is equal to 512
                            if(dataSize==512&&i==ctx->windowSize)
                            {
                                break;
                            }
                            //if it is the last packet of this window,keep transmission ON
                            //transmit a NULL Packet 
                            ctx->isTransmissionON=false;
                            auto pr1=build_data_packet(ctx);
                            Packet nullPacket=pr1.first;
                            // ctx->window.push_back(nullPacket);
                            ctx->window.insert(nullPacket);

                            break;
                        }

                        //update cur_ack_no
                        ctx->cur_ack_no++;
                    }

                    //update str blk pointer
                    ctx->str_block_ptr=ctx->cur_block_ptr;
                    //reset cur_ack_no
                    ctx->cur_ack_no=ctx->startSEQN;
                }
                mtx_window.unlock();

                //signal the sender thread to transmit the window
                {
                    std::unique_lock<std::mutex> lock(mtx);

                    if(ctx->should_sleep)
                    {
                        ctx->should_sleep=false;
                        cv.notify_one();
                    }
                    else
                    {
                        //prevent missed notification
                        cv.wait(lock,[ctx]{return ctx->should_sleep;});
                        ctx->should_sleep=false;
                        cv.notify_one();
                    }
                }

                cout<<"Window of packets formed. Transmission will begin...\n";
                LOG("Window of packets formed. Transmission will begin...","");

                break;
            }

            case 2:{

                //write job 

                //set mode
                ctx->mode=1;

                //extract filename from data
                //datasize->length of name of file
                std::memcpy(ctx->FILENAME,job.data,job.dataSize);
                ctx->FILENAME[job.dataSize]='\0';

                break;
            }

            case 3:{

                //this is an ACK Job

                //extract the ACK from job
                int next_ack;
                std::memcpy(&next_ack,job.data,job.dataSize);

                if(ctx->finalSEQN!=ctx->startSEQN+ctx->windowSize-1)
                {
                    ctx->finalSEQN=ctx->startSEQN+ctx->windowSize-1;
                }

                //check if the ACK is valid
                if(next_ack>=ctx->startSEQN&&next_ack<=ctx->finalSEQN)
                {
                    //ACK not in current window
                    //check if switch happened between window shift and updating of blk_ptr
                    if(ctx->cur_block_ptr!=ctx->str_block_ptr)
                    {
                        //both should be same if a Job has been correctly committed
                        ctx->str_block_ptr=ctx->cur_block_ptr;
                        //make them equal
                        //reset cur_ack_no
                        ctx->cur_ack_no=ctx->startSEQN;
                        break;
                    }

                    //this ACK is old
                    //reject this ack;
                    cout<<"Rejecting old ACK with ACK Number="<<next_ack<<"\n";
                    LOG("Rejecting old ACK with ACK Number=",to_string(next_ack));
                    break;
                }

                if(!ctx->isTransmissionON)
                {
                    //this is the final ACK
                    cout<<"Final Ack has been received.\n";
                    LOG("Final Ack has been received.","");
                    cout<<"Transfer of "<<formatSize(getFileSize(ctx))<<" complete.\n";
                    LOG("Transfer of ",formatSize(getFileSize(ctx))+" complete.\n");
                    resetDS(ctx);
                    break;
                }

                //reset the current window
                if(!ctx->window.empty())
                {
                    //makes sure that this is not a repeated job
                    resetWindow(ctx);
                    cout<<ctx->cur_block_ptr<<"/"<<getFileSize(ctx)<<" bytes transferred to client.\n";
                    double percentage=((double)ctx->cur_block_ptr/(double)getFileSize(ctx))*100.0;
                    cout<<"File transfer progress:"<<percentage<<"%\n";

                    LOG("File transfer progress:",to_string(percentage)+"%");
                }

                //reset cur_ack_no
                ctx->cur_ack_no=(ctx->startSEQN+ctx->windowSize)%ctx->SEQN;

                //reset block pointer
                ctx->cur_block_ptr=ctx->str_block_ptr;

                mtx_window.lock();
                {

                    //form the new window of packets
                    for(int i=1;i<=ctx->windowSize;i++)
                    {
                        auto pr=build_data_packet(ctx);
                        size_t dataSize=pr.second;
                        Packet packet=pr.first;
                        // ctx->window.push_back(packet);
                        ctx->window.insert(packet);

                        //check if last block has been read
                        if(ctx->cur_block_ptr>=getFileSize(ctx))
                        {
                            //if the data in the packet is less than 512 
                            if(dataSize<512)
                            {
                                ctx->isTransmissionON=false;
                                break;
                            }

                            //if the data in the packet is equal to 512
                            //if it is the last packet of this window,keep transmission ON
                            //i will try to send NULL packet in next window and terminate 
                            if(dataSize==512 && i==ctx->windowSize)
                            {
                                break;
                            }
                            
                            //transmit a NULL Packet 
                            ctx->isTransmissionON=false;
                            auto pr1=build_data_packet(ctx);
                            Packet nullPacket=pr1.first;
                            // ctx->window.push_back(nullPacket);
                            ctx->window.insert(nullPacket);
                            
                            break;
                        }

                        //update cur_ack_no
                        ctx->cur_ack_no++;
                    }

                    //update the window sequence numbers
                    ctx->startSEQN=(ctx->startSEQN+ctx->windowSize)%ctx->SEQN;
                    ctx->finalSEQN=(ctx->finalSEQN+ctx->windowSize)%ctx->SEQN;

                    //update str blk pointer
                    ctx->str_block_ptr=ctx->cur_block_ptr;
                    //reset cur_ack_no
                    ctx->cur_ack_no=ctx->startSEQN;
                }
                mtx_window.unlock();

                //signal the sender thread to transmit the window
                {
                    std::unique_lock<std::mutex> lock(mtx);

                    if(ctx->should_sleep)
                    {
                        ctx->should_sleep=false;
                        cv.notify_one();
                    }
                    else
                    {
                        //prevent missed notification problem
                        cv.wait(lock,[ctx]{return ctx->should_sleep;});
                        ctx->should_sleep=false;
                        cv.notify_one();
                    }
                }

                break;
            }

            case 4:{

                //this is a NACK job
                int req_nack;
                std::memcpy(&req_nack,job.data,job.dataSize);

                if(req_nack>=ctx->startSEQN&&req_nack<=ctx->finalSEQN)
                {
                    //check if the NACK is valid
                    //search in the window of packets and only send that singular packet
                    handleNackJob(req_nack,sockfd,ctx);
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
                std::memcpy(&ackno,job.data,sizeof(int));
                std::memcpy(&bytesToWrite,job.data+4,sizeof(int));
                std::memcpy(buffer,job.data+8,bytesToWrite);

                Packet packet(ackno,buffer,bytesToWrite);

                if(ctx->finalSEQN!=ctx->startSEQN+ctx->windowSize-1)
                {
                    ctx->finalSEQN=ctx->startSEQN+ctx->windowSize-1;
                }
                bool inWindow=(ackno>=ctx->startSEQN&&ackno<=ctx->finalSEQN);

                //check if the data has been retransmitted by the sender
                //ack was lost, retransmit previous ack
                if(!inWindow)
                {
                    //check if switch occurred between window shift and updating
                    //of blk_ptr
                    if(ctx->str_block_ptr!=ctx->cur_block_ptr)
                    {
                        //the same job is being processed

                        //transmit an ACK
                        Packet ACKpacket=build_ack_packet(ctx->startSEQN,ctx);
                        transmitACK(ACKpacket,sockfd,ctx);
                        cout<<"Transmitted ACK="<<ctx->startSEQN<<"\n";
                        LOG("Transmitted ACK=",to_string(ctx->startSEQN));

                        //clear the window(transmission) as well
                        resetReceiverWindow(ctx);

                        //equalise blk_ptrs
                        ctx->str_block_ptr=ctx->cur_block_ptr;

                        break;
                    }

                    Packet ACKpacket=build_ack_packet(ctx->startSEQN,ctx);
                    transmitACK(ACKpacket,sockfd,ctx);
                    cout<<"Transmitted ACK="<<ctx->startSEQN<<"\n";
                    LOG("Transmitted ACK=",to_string(ctx->startSEQN));

                    cout<<"Rejecting old DATA packet with ACK Number="<<ackno<<"\n";
                    LOG("Rejecting old DATA packet with ACK Number=",to_string(ackno));
                    break;
                }

                //if data belongs to current window insert into receiver window
                {
                    mtx_receiverWindow.lock();

                    ctx->receiverWindow.insert(packet);

                    //if the receiver window size is full
                    if(ctx->receiverWindow.size()==ctx->windowSize)
                    {
                        ctx->cur_block_ptr=ctx->str_block_ptr;

                        //->write all the packets to file using helper function
                        //receiver window must be cleared

                        for(auto &dataPacket:ctx->receiverWindow)
                        {
                            cout<<"Writing data packet with ackno="<<dataPacket.ackno<<"\n";
                            ssize_t bytesWritten=writeToFile(dataPacket.data,dataPacket.dataSize,ctx);
                        }

                        //update seqn numbers
                        ctx->startSEQN=(ctx->startSEQN+ctx->windowSize)%ctx->SEQN;
                        ctx->finalSEQN=(ctx->finalSEQN+ctx->windowSize)%ctx->SEQN;

                        //->transmit an ACk
                        Packet ACKpacket=build_ack_packet(ctx->startSEQN,ctx);
                        transmitACK(ACKpacket,sockfd,ctx);

                        //clear the window(transmission) as well
                        resetReceiverWindow(ctx);

                        //reset blk_ptr
                        ctx->str_block_ptr=ctx->cur_block_ptr;
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
                memset(buffer,'\0',BUFFER_SIZE);

                int ackno;
                int bytesToWrite;
                memset(buffer,'\0',BUFFER_SIZE);
                std::memcpy(&ackno,job.data,sizeof(int));
                std::memcpy(&bytesToWrite,job.data+4,sizeof(int));
                std::memcpy(buffer,job.data+8,bytesToWrite);

                Packet packet(ackno,buffer,bytesToWrite);

                bool inWindow=(ackno>=ctx->startSEQN&&ackno<=ctx->finalSEQN);

                //check if the data has been retransmitted by the sender
                //ack was lost, retransmit previous ack
                if(!inWindow)
                {
                    //check if switch occurred between the window shift and updation
                    //of blk pointers
                    if(ctx->str_block_ptr!=ctx->cur_block_ptr)
                    {
                        //same Job is being processed
                        //send final ACK
                        Packet ACKpacket=build_ack_packet(ctx->startSEQN,ctx);
                        transmitACK(ACKpacket,sockfd,ctx);
                        cout<<"Final ACK transmitted.\n";
                        LOG("Final ACK transmitted.","");

                        //reset receiver window
                        resetReceiverWindow(ctx);

                         //also should terminate the client but after a threshold period
                        setTime = std::chrono::steady_clock::now();  // Store time of last packet
                        ctx->isTransmissionON = false;  // Set transmission flag to false (end transmission)

                        //update blk ptr
                        ctx->str_block_ptr=ctx->cur_block_ptr;
                        break;
                    }

                    Packet ACKpacket=build_ack_packet(ctx->startSEQN,ctx);
                    transmitACK(ACKpacket,sockfd,ctx);
                    //also handles the case if the last ACK is lost
                    cout<<"Rejecting old DATA packet with ACK Number="<<ackno<<"\n";
                    LOG("Rejecting old DATA packet with ACK Number=",to_string(ackno));
                    break;
                }

                //this is the last data packet of transmission in current Window
                //insert it into receiver window
                {
                    mtx_receiverWindow.lock();

                    ctx->receiverWindow.insert(packet);

                    //reset blk ptr
                    ctx->cur_block_ptr=ctx->str_block_ptr;

                    //write to File all the packets received
                    for(auto &dataPacket:ctx->receiverWindow)
                    {
                        cout<<"Writing data packet with ackno="<<dataPacket.ackno<<"\n";
                        ssize_t bytesWritten=writeToFile(dataPacket.data,dataPacket.dataSize,ctx);
                    }

                    //update seqn numbers
                    ctx->startSEQN=(ctx->startSEQN+ctx->windowSize)%ctx->SEQN;
                    ctx->finalSEQN=(ctx->finalSEQN+ctx->windowSize)%ctx->SEQN;

                    //send final ACK
                    Packet ACKpacket=build_ack_packet(ctx->startSEQN,ctx);
                    transmitACK(ACKpacket,sockfd,ctx);
                    cout<<"Final ACK transmitted.\n";
                    LOG("Final ACK transmitted.","");

                    //reset receiver window
                    resetReceiverWindow(ctx);

                    //also should terminate the client but after a threshold period
                    setTime = std::chrono::steady_clock::now();  // Store time of last packet
                    ctx->isTransmissionON = false;  // Set transmission flag to false (end transmission)

                    //update blk ptr
                    ctx->str_block_ptr=ctx->cur_block_ptr;

                    mtx_receiverWindow.unlock();
                }

                break;
            }

            case 7:{

                //this job is to handle error

                cout<<"Error has occurred at the client side.Ending transmission..."<<"\n";
                resetDS(ctx);

                break;
            }
            default:{

                cout<<"Wrong JobId. Skipping Job"<<endl;

                break;
            }
        }

        mtx_jobQueue.lock();
        if(!ctx->jobQueue.empty()) ctx->jobQueue.pop_front();
        mtx_jobQueue.unlock();
    }
}