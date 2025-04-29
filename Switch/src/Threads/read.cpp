#include "Globals.hpp"

void reader_thread(int sockfd){

    //setup
    sockaddr_in sender_addr;
    socklen_t sender_len = sizeof(sender_addr);
    u_char buffer[BUFFER_SIZE];
    char sender_ip[INET_ADDRSTRLEN];
    int sender_port;
    std::string sender_key ,fileName;
    std::string the_actual_client_key;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    size_t total = 0, dropped = 0;

    while (true) {
        // read from socket()
        bzero(buffer,BUFFER_SIZE);
        ssize_t received_bytes = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&sender_addr, &sender_len);
        check_err(received_bytes,"recvfrom failed");
        buffer[received_bytes] = '\0';  // Null-terminate the received data

        // Convert senders address to a readable IP and port
        bzero(sender_ip,INET_ADDRSTRLEN);
        inet_ntop(AF_INET, &sender_addr.sin_addr, sender_ip, INET_ADDRSTRLEN);
        sender_port = ntohs(sender_addr.sin_port);

        // Create a unique key associated with every sender
        sender_key = std::string(sender_ip) + "_" + std::to_string(sender_port);

        //debug::
        ////std::cout<<" Just received a packet from "<<sender_key<<" and start decoding his packet\n";

        // selecting Active Server
        mtx_ServerList.lock();
        std::string curr_active = server_list[0];
        std::string curr_backup = server_list[1];
        mtx_ServerList.unlock();

        if(sender_key == curr_active){ // recv mssg is from server

            mtx_wd.lock();
            watchDogCnt=0;
            mtx_wd.unlock();

            //std::cout<<"Reset WD cnt\n";
            // Decide whether to drop packet or NOT [just like epsilon-greedy]
            if(buffer[1]==3){ // data packet
                float r = dis(gen); // Generate [0.0, 1.0) from uniform distribution
                //total++;
                if (r < packet_drop_prob){ // Drop current data Packet
                    // dropped++;
                    // if (total % 1 == 0) {
                    //     //std::cout << "[DEBUG] Drop Rate: " << (100.0 * dropped / total) << "%\n";
                    //     //std::cout <<"[DEBUG] total = "<<total<<" dropped = "<<dropped<<"\n";
                    //     //std::cout << "Server data packet dropped\n";
                    // }
                    continue;
                }
            }


            //create job
            job* thejob = new job;
            thejob->destn_type = 'C';
            thejob->destn = the_actual_client_key;
            thejob->packet_size = received_bytes;
            thejob->packet = (char *)malloc(thejob->packet_size);
            memcpy((void *)thejob->packet, buffer, thejob->packet_size);


            //push job to WorkQ
            {
                std::lock_guard<std::mutex> lock2(mtx_WorkQ);
                WorkQ.push_back(thejob);
            }
            ////std::cout<<"Mssg recv from Active server | Job created and Push to WorkQ\n";

        }
        else if(sender_key != curr_backup){// recv mssg is from client

            the_actual_client_key = sender_key;

            // Decide whether to drop packet or NOT [just like epsilon-greedy]
            if(buffer[1]==3){ // data packet
                float r = dis(gen); // Generate [0.0, 1.0) from uniform distribution
                //total++;
                if (r < packet_drop_prob){ // Drop current data Packet
                    // dropped++;
                    // if (total % 1 == 0) {
                    //     //std::cout << "[DEBUG] Drop Rate: " << (100.0 * dropped / total) << "%\n";
                    //     //std::cout <<"[DEBUG] total = "<<total<<" dropped = "<<dropped<<"\n";
                    //     //std::cout <<"Client Data packet drop\n";
                    // }
                    continue;
                }
            }

            // create job;
            job* thejob = new job;
            thejob->destn_type = 'S';
            thejob->destn = curr_active;
            thejob->packet_size = received_bytes;
            thejob->packet = (char *)malloc(thejob->packet_size);
            memcpy((void *)thejob->packet, buffer, thejob->packet_size);

            //push job to WorkQ
            {
                std::lock_guard<std::mutex> lock2(mtx_WorkQ);
                WorkQ.push_back(thejob);
            }
            ////std::cout<<"Mssg recv from client | Job created and Push to WorkQ\n";
        }

        cv_work.notify_one(); // notifies the frwd thread that WorQ has data
    }// end of while true
}//end of read thread
