#include "Globals.hpp"

void frwd_thread(int sockfd){
    //setup
    job* thejob;
    sockaddr_in client_addr;

    while(true){

        // Remove front job from the Work Q
        {
            std::unique_lock<std::mutex> lock2(mtx_WorkQ);
            cv_work.wait(lock2, [] { return !WorkQ.empty(); });

            thejob = WorkQ.front();
            WorkQ.pop_front();
        }

        //debug lines
        //std::cout<<"Inside Frwd Thread After Extract Job\n";
        //thejob->display_job();

        // Parse the IP and port from the client key
        size_t pos = thejob->destn.find('_');
        std::string client_ip = thejob->destn.substr(0, pos);
        int client_port = std::stoi(thejob->destn.substr(pos + 1));

        // Set up the client address structure
        memset(&client_addr, 0, sizeof(client_addr));
        client_addr.sin_family = AF_INET;
        inet_pton(AF_INET, client_ip.c_str(), &client_addr.sin_addr);
        client_addr.sin_port = htons(client_port);

        //sending Packets to Respective Destination
        ssize_t sent_bytes = sendto(sockfd, thejob->packet, thejob->packet_size, 0, (const struct sockaddr*)&client_addr, sizeof(client_addr));
        check_err(sent_bytes,"sendto failed");
        free(thejob->packet);
        //std::cout<<"Packet Sent to "<<thejob->destn<<"\n";

        //WatchDog - HeartBeat Check
        bool switchFlag = false;
        if(thejob->destn_type == 'S'){

            ////std::cout<< " Try to Aquire context lock\n";

            mtx_wd.lock();
            //std::cout<<"Checking For Switch wdCnt = "<<watchDogCnt<<"\n";
            watchDogCnt++;// means server is still on if > threshold ==> server off
            if((watchDogCnt) > threshold){
                switchFlag = true;
                watchDogCnt =0; // once switch reset counter for new server
                //std::cout<<"SWITCH OCCURS\n";
            }
            mtx_wd.unlock();

        }

        if(switchFlag){ // switching active and backup servers
            std::string curr_active;
            mtx_ServerList.lock();
            //std::cout<<"Swapping Done \n";
            std::swap(server_list[0],server_list[1]);
            curr_active = server_list[0];
            mtx_ServerList.unlock();

            // traverse through the WorkQ and update the destn for type ="S"
            {
                std::lock_guard<std::mutex> lock2(mtx_WorkQ);
                //std::cout<<"Changing Address of server to backup in WorkQ \n";
                for(auto &el : WorkQ){
                    if(el->destn_type=='S'){
                        el->destn = curr_active;
                    }
                }

            }
            std::cout<<"Signal Backup\n";
            sem_post(sem);  // Signal the consumer[BackUp seerver]
        }

        delete thejob;
    }// end of inf loop
}
