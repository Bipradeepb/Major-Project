#pragma once

#include "std_headers.h"
#include "Logger.hpp"

////////////////////////////////////////// contents of Worker Queue

class job{
public:

    char destn_type; // S for server | C for client
    std::string destn; // Ip+"_"+Port
    u_char * packet;//go-back-n packet
    int packet_size;

    void display_job(){
        LOG("dest_type = ",destn_type,"\n");
        LOG("destn = ",destn,"\n");
        // Output the resulting string in hex for verification
        LOG("1st 4 bytes of Packet in hex form \n");
        LOG((uint)packet[0]," ",(uint)packet[1]," ",(uint)packet[2]," ",(uint)packet[3],"\n");
        LOG("packet_size = ",packet_size,"\n");
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
inline float packet_drop_prob = 0.0f;

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
