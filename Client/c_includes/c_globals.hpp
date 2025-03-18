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
    char* data;
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