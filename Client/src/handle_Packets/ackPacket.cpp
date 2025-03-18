#include "globals.hpp"
/*

          2 bytes    2 bytes
          -------------------
   ACK   | 04    |   Block #  |
          --------------------

            n bytes    1byte     2 bytes    2 bytes
            -----------------------------------------
sent ACK   |Client_id |   0    | 04    |   Block #  |
            ------------------------------------------

*/

// Function to build ACK packet prepend with client_id
std::pair<unsigned char*, int> build_ack_packet(const std::string& id, int block_number) {
    // Step 1: Create a clean ID without any trailing null characters
    std::string clean_id = id;
    while (!clean_id.empty() && clean_id.back() == '\0') {
        clean_id.pop_back(); // Remove trailing null characters if any
    }

    // Step 2: Calculate total packet size: ID length + 1 (for single null terminator) + 4 (ACK packet)
    int packet_size = clean_id.size() + 1 + 4;
    unsigned char* packet = (unsigned char*)malloc(packet_size);

    // Step 3: Copy ID into the packet
    memcpy(packet, clean_id.c_str(), clean_id.size());
    
    // Step 4: Append a single null character after ID
    packet[clean_id.size()] = 0x00;

    // Step 5: Append ACK opcode and block number
    packet[clean_id.size() + 1] = 0x00;          // Opcode high byte
    packet[clean_id.size() + 2] = 0x04;          // Opcode low byte (ACK)
    packet[clean_id.size() + 3] = block_number >> 8; // Block number high byte
    packet[clean_id.size() + 4] = block_number & 0xFF; // Block number low byte

    // Return the packet and its size
    return {packet, packet_size};
}

// Function to extract fields from ACK packet
ACK_Packet extract_ack_packet(const unsigned char* packet) {
    ACK_Packet ack_packet;
    ack_packet.block_number = (packet[2] << 8) | packet[3];
    return ack_packet;
}


