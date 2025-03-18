#pragma once //prevent multiple inclusion of same header in a single translation unit

#include "globals.hpp"
//////////////////////////////////////////////////////////////////////////////////////////////////

//read_write packet
unsigned char* build_rrq_wrq_packet(const char* filename, int WinSize, int opcode);
RRQ_WRQ_Packet extract_rrq_wrq_packet(const unsigned char* packet);

//data packet
unsigned char* build_data_packet(int block_number, const unsigned char* data, int data_size);
DATA_Packet extract_data_packet(const unsigned char* packet, int packet_size);

//ack packet
unsigned char* build_ack_packet(int block_number);
ACK_Packet extract_ack_packet(const unsigned char* packet);


///////////////////////////////////////////////////////////////////////////////////////////





