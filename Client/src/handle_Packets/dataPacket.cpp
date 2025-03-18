#include "globals.hpp"
/*
          2 bytes    2 bytes       n bytes
          ---------------------------------
   DATA  | 03    |   Block #  |    Data    |
          ---------------------------------

        n bytes      1bytes     2 bytes    2 bytes       n bytes
sent    ---------------------------------------------------------
DATA    |client_id |     0    | 03    |   Block #  |    Data    |
        --------------------------------------------------------         


*/

// Function to build DATA packet
std::pair<unsigned char*, int> build_data_packet(const std::string& id, int block_number, const unsigned char* data, int data_size) {
    // Step 1: Create a clean ID without any trailing null characters
    std::string clean_id = id;
    while (!clean_id.empty() && clean_id.back() == '\0') {
        clean_id.pop_back(); // Remove trailing null characters if any
    }

    // Step 2: Calculate total packet size: ID length + 1 (for single null terminator) + 4 (DATA header) + data_size
    int packet_size = clean_id.size() + 1 + 4 + data_size;
    unsigned char* packet = (unsigned char*)malloc(packet_size);

    // Step 3: Copy ID into the packet
    memcpy(packet, clean_id.c_str(), clean_id.size());

    // Step 4: Append a single null character after ID
    packet[clean_id.size()] = 0x00;

    // Step 5: Append DATA opcode and block number
    packet[clean_id.size() + 1] = 0x00;           // Opcode high byte
    packet[clean_id.size() + 2] = 0x03;           // Opcode low byte (DATA)
    packet[clean_id.size() + 3] = block_number >> 8; // Block number high byte
    packet[clean_id.size() + 4] = block_number & 0xFF; // Block number low byte

    // Step 6: Append data payload
    memcpy(packet + clean_id.size() + 5, data, data_size);

    // Return the packet and its size
    return {packet, packet_size};
}

// Function to extract fields from DATA packet
DATA_Packet extract_data_packet(const unsigned char* packet, int packet_size) {
    DATA_Packet data_packet;
    data_packet.block_number = (packet[2] << 8) | packet[3];
    data_packet.data_size = packet_size - 4; // Exclude opcode and block number bytes
    data_packet.data = (char*)malloc(data_packet.data_size);
    memcpy(data_packet.data, packet + 4, data_packet.data_size);
    return data_packet;
}


