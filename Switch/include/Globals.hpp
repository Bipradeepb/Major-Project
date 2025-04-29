#pragma once

#include "std_headers.h"


////////////////////////////////////////// contents of Worker Queue

class job{
public:

    char destn_type; // S for server | C for client
    std::string destn; // Ip+"_"+Port
    char * packet;// std-tftp packet
    int packet_size;

    void display_job(){
        std::cout<<"dest_type = "<<destn_type<<"\n";
        std::cout<<"destn = "<<destn<<"\n";
        // Output the resulting string in hex for verification
        std::cout<<"Packet in hex form \n";
        for (int i =0;i<packet_size;++i) {
            printf("%02X ", packet[i]);
        }
        std::cout << "\n";
        std::cout<<"packet_size = "<<packet_size<<"\n";
    }
    ~job(){;} // to prevent free the dynamic memory by default destructor
};


///////////////////////////////////////////// DS and variables //////////////////////

inline int threshold;//Max Number of retransmission allowed beyond which server is declare dead

inline int watchDogCnt{0}; // measure of how many packets are sent to server and havent recv any reply

inline std::list<job *> WorkQ;

//treat server_list[0] is active server
inline std::string server_list[2];//List of Load_Balancer

inline std::mutex mtx_WorkQ, mtx_ServerList , mtx_wd;
inline std::condition_variable cv_work;// Condition variable to notify tfrwd thread

inline int sw_port;

///  for tcp connection of backup
inline int server_fd, client_socket = -1;
inline std::mutex mtx;
inline std::condition_variable cv;
inline bool allow_next_accept = false;

/////////////////////////////// Helper Functions ////////////////////

inline void check_err(int fd,std::string mssg){
	if(fd <0){
		perror(mssg.c_str());
		exit(1);
	}
}
