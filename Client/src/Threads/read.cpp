#include "globals.hpp"
#include "packets.hpp"


std::tuple<std::string, std::string , const unsigned char * , int> extractPrefix(const unsigned char* data) {
    // Step 1: Convert unsigned char* to char* for easier handling
    const char* ptr = reinterpret_cast<const char*>(data);
    int pad =0;
    // Step 2: Extract ID (up to the first '\0')
    std::string id(ptr);
    pad+=id.size() + 1;
    ptr += id.size() + 1; // Move pointer past the ID and '\0'


    // Step 3: Extract FileName (up to the next '\0')
    std::string fileName(ptr);
    pad+=fileName.size() + 1;
    ptr += fileName.size() + 1; // Move pointer past the FileName and '\0'
    

    return {id, fileName ,reinterpret_cast<const unsigned char*>(ptr),pad};
}


void reader_thread(int sockfd){

    //setup
    u_char buffer[BUFFER_SIZE];
    socklen_t client_len;
    ssize_t received_bytes;

    while (true) {

        // read from socket()
        bzero(buffer,BUFFER_SIZE);

        // Set up the file descriptor set
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);

        // Set up the timeout structure
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        // Use select to wait for the socket to be ready for reading
        int select_result = select(sockfd + 1, &readfds, NULL, NULL, &timeout);

        if (select_result < 0) {
            // An error occurred with select
            perror("select error");
            continue;
        } else if (select_result == 0) {
            // Timeout occurred, no data to read
            printf("recvfrom timed out\n");
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            continue;
        } else {
            mtx_cli_addr.lock();
            client_len = sizeof(client_addr);
            received_bytes = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &client_len);            
            mtx_cli_addr.unlock();
            check_err(received_bytes,"recvfrom failed");
            buffer[received_bytes] = '\0';  // Null-terminate the received data
        }
        
        //read from socket finished
        
        // //debug:
        // std::cout<<"Just Recv a packet = \n";
        // for (int i =0; i<received_bytes;++i) {
        //     printf("%02X ", buffer[i]);
        // }
        // std::cout << "\n";

        //As the mssg is coming from switch/LB it has prefix data on std tftp
        std::cout<<"Start Extracting Prefix from recv packet\n";
        auto [client_key, fileName, tftp ,prefix_pad ] = extractPrefix(buffer);
        //tftp is the  ptr to start of std tftp packet

        //debug::
        std::cout<<" Result of Extarction:-\n Rceived the packet from "<<client_key<<"\n";
        std::cout<<" prefix_pad_size = "<<prefix_pad<<" fileName = "<<fileName<<"\n";
        std::cout<<"Printing the  Tftp  extract\n";
        for (int i =0; i<received_bytes-prefix_pad;++i) {
            printf("%02X ", tftp[i]);
        }
        std::cout << "\n";


        //initialising the context[client_id v/s curr_blk]
        mtx_context.lock();
        if(client_context.find(client_key) == client_context.end()){// not found
            client_context[client_key]=-1;// setting current block to -1;
        }
        mtx_context.unlock();
        

        // decode the received tftp packet
        switch(tftp[1]){
            case 0x01: // Read Packet
            case 0x02: // Write Packet
            {
                    
                    // create job for Work Queue
                    job thejob;
                    if(tftp[1] == 0x01) thejob.type = 'R';
                    if(tftp[1] == 0x02) thejob.type = 'W';
                    thejob.block_num = 0;
                    thejob.mssg_size = 0;
                    thejob.client_id = client_key;
                    thejob.fileName = fileName;
                    thejob.message ="";
                    thejob.timestamp = std::chrono::steady_clock::time_point::min();
                   
                    // update the global data structure client_context                        
                    mtx_context.lock();
                    // if 1st read/wr mssg from client (then curr_block should be -1 and thejob.blk =0)
                    if(thejob.block_num > client_context[client_key]){
                        client_context[client_key] = thejob.block_num;

                        mtx_context.unlock();
                        // update the global data structure WorkQueue
                        {
                            std::lock_guard<std::mutex> lock2(mtx_WorkQ);
                            WorkQ.push_back(thejob);
                        }

                        //debug:
                        std::cout<<"Inside Reader Thread Added R/W job to workQ\n";
                    }
                    else{
                        mtx_context.unlock(); 
                        // context for client_key exists and recv is RD/WR 
                        //=> duplicate mssg from client recived
                    }

            }
                    break;
            case 0x03:
            {
                    DATA_Packet  data_packet  = extract_data_packet(tftp, received_bytes - prefix_pad);

                    // create job for Work Queue;
                    job thejob;
                    thejob.type ='D';
                    thejob.block_num = data_packet.block_number;
                    thejob.client_id = client_key;
                    thejob.fileName =  fileName;                   
                    thejob.message = std::string(data_packet.data);
                    thejob.mssg_size = data_packet.data_size;
                    thejob.timestamp = std::chrono::steady_clock::time_point::min();
                
                    // the global data structure:- access client_context | update TimerQ WorkQ                                      
                    mtx_context.lock();
                    //recv block_num > present block_num  ==> new block recv
                    if(thejob.block_num > client_context[client_key]){
                        
                        //update  client_Context
                        client_context[client_key] = thejob.block_num;
                        mtx_context.unlock();

                        {// update Work Q
                        std::lock_guard<std::mutex> lock2(mtx_WorkQ);
                        WorkQ.push_back(thejob);}

                        //debug:
                        std::cout<<"Inside Reader Thread Added D job to workQ\n";

                        {// [Remove entry from timer if new mssg recev before timer expire]
                        std::lock_guard<std::mutex> lock3(mtx_Timer);
                        if(TimerM.find(client_key)!= TimerM.end()){
                            //if(std::chrono::duration_cast<std::chrono::milliseconds>(recv_timestamp-TimerM[client_key]->timestamp) < threshold ){
                                TimerQ.erase(TimerM[client_key]);
                                TimerM.erase(client_key);
                            //}
                        } }

                    }else{
                        mtx_context.unlock();
                        // The message received is a repeated one : do nothing
                    }
                         
                    //std::cout<<"Inside Read Thread : D case release lock\n";
                    //cleanup:
                    free(data_packet.data);


            }
                    break;
            case 0x04:
            {
                    ACK_Packet the_ack_packet = extract_ack_packet(tftp);
                    
                    // create job for Work Queue;
                    job thejob;
                    thejob.type ='A';
                    thejob.block_num = the_ack_packet.block_number;
                    thejob.fileName = fileName;
                    thejob.client_id = client_key;
                    thejob.message = "";
                    thejob.mssg_size = 0;
                    thejob.timestamp = std::chrono::steady_clock::time_point::min();
                    
                    // the global data structure:- access client_context | update TimerQ WorkQ 

                    mtx_context.lock();

                    //recv block_num > present block_num  ==> new block recv
                    if(thejob.block_num > client_context[client_key]){
                        
                        //update  client_Context
                        client_context[client_key] = thejob.block_num;
                        mtx_context.unlock();

                        {// update Work Q
                        std::lock_guard<std::mutex> lock2(mtx_WorkQ);
                        WorkQ.push_back(thejob);}

                        //debug:
                        std::cout<<"Inside Reader Thread Added A job to workQ\n";

                        {// [Remove entry from timer if new mssg recev before timer expire]
                        std::lock_guard<std::mutex> lock3(mtx_Timer);
                        if(TimerM.find(client_key)!= TimerM.end()){
                            //if(std::chrono::duration_cast<std::chrono::milliseconds>(recv_timestamp-TimerM[client_key]->timestamp) < threshold ){
                                TimerQ.erase(TimerM[client_key]);
                                TimerM.erase(client_key);
                            //}
                        }}

                    }else{
                        mtx_context.unlock();
                        // The message received is a repeated one : do nothing
                    }
                    
            }
                    break;
            case 0x05:
                    std::cout<<"Received error message from client\n";
                    break;
            default :
                    std::cout<<" Received Wrong Packet Opcode\n";
                    exit(0);
        }
        
        cv_work.notify_one(); // notifies the frwd thread that WorQ has data

    }// inf looping while(true)


}