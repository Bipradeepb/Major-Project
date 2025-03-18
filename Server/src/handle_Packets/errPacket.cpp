#include "globals.hpp"
/*
          2 bytes  2 bytes        string    1 byte
          ----------------------------------------
   ERROR | 05    |  ErrorCode |   ErrMsg   |   0  |
   	  ----------------------------------------
*/



// Function to build ERROR packet
unsigned char* build_error_packet(int error_code, const char* error_message) {
    int message_length = strlen(error_message);
    unsigned char* packet = (unsigned char*)malloc(5 + message_length);
    packet[0] = 0x00; // Opcode high byte
    packet[1] = 0x05; // Opcode low byte (ERROR)
    packet[2] = error_code >> 8; // Error code high byte
    packet[3] = error_code & 0xFF; // Error code low byte
    strcpy((char*)(packet + 4), error_message);
    packet[4 + message_length] = 0x00; // Null terminator
    return packet;
}

// Function to extract fields from ERROR packet
ERROR_Packet extract_error_packet(const unsigned char* packet, int packet_size) {
    ERROR_Packet error_packet;
    error_packet.error_code = (packet[2] << 8) | packet[3];
    error_packet.error_message = (char*)malloc(packet_size - 5); // Exclude opcode and error code bytes
    strcpy(error_packet.error_message, (const char*)(packet + 4));
    return error_packet;
}


