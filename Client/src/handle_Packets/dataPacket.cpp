#include "c_globals.hpp"
/*
          2 bytes    2 bytes       n bytes
          ---------------------------------
   DATA  | 03    |   Block #  |    Data    |
          ---------------------------------

*/

// Function to build DATA packet
unsigned char* build_data_packet(int block_number, const unsigned char* data, int data_size) {
    unsigned char* packet = (unsigned char*)malloc(data_size + 4);
    packet[0] = 0x00; // Opcode high byte
    packet[1] = 0x03; // Opcode low byte (DATA)
    packet[2] = block_number >> 8; // Block number high byte
    packet[3] = block_number & 0xFF; // Block number low byte
    memcpy(packet + 4, data, data_size);
    return packet;
}

// Function to extract fields from DATA packet
DATA_Packet extract_data_packet(const unsigned char* packet, int packet_size) {
    DATA_Packet data_packet;
    data_packet.block_number = (packet[2] << 8) | packet[3];
    data_packet.data_size = packet_size - 4; // Exclude opcode and block number bytes
    data_packet.data = (u_char*)malloc(data_packet.data_size);
    memcpy(data_packet.data, packet + 4, data_packet.data_size);
    return data_packet;
}
