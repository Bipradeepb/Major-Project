#include "c_globals.hpp"
/*

          2 bytes    2 bytes
          -------------------
   ACK   | 04    |   Block #  |
          --------------------

*/

// Function to build ACK packet
unsigned char* build_ack_packet(int block_number) {
    unsigned char* packet = (unsigned char*)malloc(4);
    packet[0] = 0x00; // Opcode high byte
    packet[1] = 0x04; // Opcode low byte (ACK)
    packet[2] = block_number >> 8; // Block number high byte
    packet[3] = block_number & 0xFF; // Block number low byte
    return packet;
}

// Function to extract fields from ACK packet
ACK_Packet extract_ack_packet(const unsigned char* packet) {
    ACK_Packet ack_packet;
    ack_packet.block_number = (packet[2] << 8) | packet[3];
    return ack_packet;
}


