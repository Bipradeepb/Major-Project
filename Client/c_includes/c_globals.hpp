#pragma once
#include "c_std_headers.h"

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

/////////////////////// Stores Info of Config File of Client
struct Config {
    std::string serverIp;
    int serverPort;
    int serverWindowSize;
    char choice;
    std::string filePath;
};

/////////////////////////////// Helper Functions ////////////////////

inline void check_err(int fd,std::string mssg){
	if(fd <0){
		perror(mssg.c_str());
		exit(1);
	}
}

inline int TIMEOUT_MILLI_SEC=100;
inline std::mutex mtx; //supports the below variables
inline  bool flag_Ack_Recv = false;
inline  bool read_thread_end = false;
inline  int last_D_blk{INT_MAX}; // used in Writer to signal Read and Write thread
inline  int current_blk = 0;
