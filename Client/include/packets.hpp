#pragma once //prevent multiple inclusion of same header in a single translation unit

#include "globals.hpp"
//////////////////////////////////////////////////////////////////////////////////////////////////

//read_write packet
unsigned char* build_rrq_wrq_packet(const char* filename, const char* mode, int opcode);
RRQ_WRQ_Packet extract_rrq_wrq_packet(const unsigned char* packet);

//data packet
std::pair<unsigned char*, int> build_data_packet(const std::string& id,int block_number, const unsigned char* data, int data_size);
DATA_Packet extract_data_packet(const unsigned char* packet, int packet_size);

//ack packet
std::pair<unsigned char*, int> build_ack_packet(const std::string& id,int block_number);
ACK_Packet extract_ack_packet(const unsigned char* packet);

//err packet
unsigned char* build_error_packet(int error_code, const char* error_message);
ERROR_Packet extract_error_packet(const unsigned char* packet, int packet_size);

///////////////////////////////////////////////////////////////////////////////////////////





