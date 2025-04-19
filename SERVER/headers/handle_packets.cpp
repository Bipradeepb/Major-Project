#pragma once
#include "helper_functions.cpp"

using namespace std;

Packet build_ack_packet(int next_ack_no,Context *ctx);
Packet build_nack_packet(int req_ack_no,Context *ctx);
pair<Packet,size_t> build_data_packet(Context *ctx);
Packet build_err_packet(Context *ctx);

char packetData[BUFFER_SIZE]; 

/*
    	-----------------------
RD  	| 01   |    Filename  |
		-----------------------

    	----------------------
WR  	| 02   |   FileName  |
		----------------------

    	---------------------
ACK  	| 03  |  next_ack#  |
		----------------------

*/

Packet build_ack_packet(int next_ack_no,Context *ctx)
{
    memset(packetData,'\0',BUFFER_SIZE);

    int opcode = htonl(3);
    // Pack Opcode (Big-Endian)
    std::memcpy(packetData, &opcode, sizeof(int));

    next_ack_no=htonl(next_ack_no);

    std::memcpy(packetData+4,&next_ack_no,sizeof(int));

	Packet packet(ctx->cur_ack_no,packetData,4);
	return packet;
}

/*
    	-----------------------
NACK 	| 04   |   Req_Ack #  |
		-----------------------

*/

Packet build_nack_packet(int req_ack_no,Context *ctx)
{
    memset(packetData,'\0',BUFFER_SIZE);

    int opcode = htonl(4);
    // Pack Opcode (Big-Endian)
    std::memcpy(packetData, &opcode, sizeof(int));

    req_ack_no=htonl(req_ack_no);

    std::memcpy(packetData+4,&req_ack_no,sizeof(int));

	Packet packet(ctx->cur_ack_no,packetData,4);
	return packet;
}


/*
        --------------------------------------------------------
DATA    | 05   |   ACK #  |   bytes sent | Data (<=512 bytes)  |
        --------------------------------------------------------

*/

pair<Packet,size_t> build_data_packet(Context* ctx)
{
    //read 512(atmost) for the current file starting at cur_block_ptr

    memset(packetData,'\0',BUFFER_SIZE);

    int opcode = htonl(5);
    // Pack Opcode (Big-Endian)
    std::memcpy(packetData, &opcode, sizeof(int));

    // Pack ACK number (Big-Endian)
    int ack=htonl(ctx->cur_ack_no);
    std::memcpy(packetData + 4, &ack, sizeof(int));

    // Read file data (at most 512 bytes)
    int bytesRead = readFromFile(packetData + 12, 512,ctx);
    bytesRead=htonl(bytesRead);
    std::memcpy(packetData+8,&bytesRead,sizeof(int));

    Packet packet(ctx->cur_ack_no,packetData,bytesRead+4);
    return {packet,bytesRead};
}


/*
    	------------------
ERR  	| 06  |  Message |
		------------------

*/

Packet build_err_packet(std:: string mssg,Context *ctx)
{
    memset(packetData,'\0',BUFFER_SIZE);

    int opcode = htonl(6);
    // Pack Opcode (Big-Endian)
    std::memcpy(packetData, &opcode, sizeof(int));

    // const char* message="Error occurred. Terminating transmission.";
    std::memcpy(packetData+4,mssg.c_str(),(size_t)mssg.length());

	Packet packet(ctx->cur_ack_no,packetData,(size_t)mssg.length());
	return packet;
}
