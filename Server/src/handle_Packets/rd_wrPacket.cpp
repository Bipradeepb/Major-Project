#include "globals.hpp"
// Function to build & extract RRQ/WRQ packet
/*
          2 bytes    string   1 byte     string   1 byte
          -----------------------------------------------
   RRQ/  | 01/02 |  Filename  |   0  |    Mode    |   0  |
   WRQ    -----------------------------------------------

*/

unsigned char* build_rrq_wrq_packet(const char* filename, const char* mode, int opcode) {
    int filename_length = strlen(filename);
    int mode_length = strlen(mode);
    unsigned char* packet = (unsigned char*)malloc(filename_length + mode_length + 4); // Adding 2 for opcode and 2 for null padding bytes
    packet[0] = opcode >> 8; // Opcode high byte
    packet[1] = opcode & 0xFF; // Opcode low byte
    strcpy((char*)(packet + 2), filename);
    packet[2 + filename_length] = 0; // Null padding byte after filename
    strcpy((char*)(packet + 3 + filename_length), mode);
    packet[3 + filename_length + mode_length] = 0; // Null padding byte after mode
    return packet;
}

// Function to extract fields from RRQ/WRQ packet
RRQ_WRQ_Packet extract_rrq_wrq_packet(const unsigned char* packet) {
    RRQ_WRQ_Packet rrq_wrq;
    //printf("Inside Extract read write extract packet \n");
    int filename_length = strlen((const char*)packet + 2);
    int mode_offset = 2 + filename_length + 1; // Adding 1 to skip null byte after filename
    int mode_length = strlen((const char*)packet + mode_offset);
	
	//printf("File Name length %d\n",filename_length);
	//printf("Mode_length %d\n",mode_length);
	
    rrq_wrq.filename = (char*)malloc(filename_length + 1); // +1 for null terminator
    strncpy(rrq_wrq.filename, (const char*)packet + 2, filename_length);
    rrq_wrq.filename[filename_length] = '\0';
	
	//printf("Extracted file name %s\n",rrq_wrq.filename);
	
    rrq_wrq.mode = (char*)malloc(mode_length + 1); // +1 for null terminator
    strncpy(rrq_wrq.mode, (const char*)packet + mode_offset, mode_length);
    rrq_wrq.mode[mode_length] = '\0';
	
	//printf("Extracted mode name %s\n",rrq_wrq.mode);
    rrq_wrq.block_number=0;
	
    return rrq_wrq;
}
