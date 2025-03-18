#include "c_globals.hpp"
// Function to build & extract RRQ/WRQ packet
/*
          2 bytes    string   1 byte     2 byte       
          -------------------------------------------
   RRQ/  | 01/02 |  Filename  |   0  |    WinSize   |
   WRQ    -------------------------------------------

*/

unsigned char* build_rrq_wrq_packet(const char* filename, int WinSize, int opcode) {
    int filename_length = strlen(filename);
    unsigned char* packet = (unsigned char*)malloc(filename_length + 5); // Adding 2 for opcode and 2 for null padding bytes
    packet[0] = opcode >> 8; // Opcode high byte
    packet[1] = opcode & 0xFF; // Opcode low byte
    strcpy((char*)(packet + 2), filename);
    packet[2 + filename_length] = 0; // Null padding byte after filename
    packet[3 + filename_length] = WinSize >>8;
    packet[4 + filename_length] = WinSize & 0xFF;
    return packet;
}

// Function to extract fields from RRQ/WRQ packet
RRQ_WRQ_Packet extract_rrq_wrq_packet(const unsigned char* packet) {
    RRQ_WRQ_Packet rrq_wrq;
    //printf("Inside Extract read write extract packet \n");
    int filename_length = strlen((const char*)packet + 2);
	
	//printf("File Name length %d\n",filename_length);
	//printf("Mode_length %d\n",mode_length);
	
    rrq_wrq.filename = (char*)malloc(filename_length + 1); // +1 for null terminator
    strncpy(rrq_wrq.filename, (const char*)packet + 2, filename_length);
    rrq_wrq.filename[filename_length] = '\0';
	
	//printf("Extracted file name %s\n",rrq_wrq.filename);
	
    rrq_wrq.WinSize = ( packet[3 + filename_length] << 8 ) | packet[4 + filename_length];
	
	//printf("Extracted mode name %s\n",rrq_wrq.mode);
    rrq_wrq.block_number=0;
	
    return rrq_wrq;
}
