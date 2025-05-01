#pragma once
#include "std_headers.h"

/////////////////////////////////// Types of TFTP Packets
class RRQ_WRQ_Packet{
public:
    int block_number;
    char* filename;
    int WinSize;
} ;

class DATA_Packet{
public:
    int block_number;
    u_char* data;
    int data_size;
} ;

class ACK_Packet{
public:
    int block_number;
} ;

/////////////////////// Stores Context in Shared Memory for Active/Backup
struct Context {
    char clientIp[16];  // IPv4 Address (e.g., "192.168.1.1")
    int clientPort;
    int WindowSize;
    int current_blk;
    char fileName[256]; // Filename (Assuming max 256 chars)
    char choice; // R if client wants to Read else W for Write
    int curr_Win; // Used only in Reader [Denotes Num of Blks Left before ACK]
};


/////////////////////////////// Helper Functions ////////////////////

inline void check_err(int fd,std::string mssg){
	if(fd <0){
		perror(mssg.c_str());
		exit(1);
	}
}

inline int TIMEOUT_MILLI_SEC{100};
inline std::mutex mtx;  // Mutex for synchronizing access to current_blk
inline bool flag_Ack_Recv = false;// used in Writer to signal Read and Write thread
inline std::atomic<bool> stop_flag{false}; // used in Writer to signal Read and Write thread
