#pragma once
#include "helper_functions.cpp"

using namespace std;

Packet* build_read_packet();
Packet* build_write_packet();
Packet* build_ack_packet(int next_ack_no);
Packet* build_nack_packet(int req_ack_no);
pair<Packet*,size_t> build_data_packet();
Packet* build_err_packet();

char packetData[BUFFER_SIZE];

/*
    	-----------------------
RD  	| 01   |    Filename  |
		-----------------------

*/

Packet* build_read_packet()
{
    memset(packetData,'\0',BUFFER_SIZE);

    int opcode = htonl(1);
    // Pack Opcode (Big-Endian)
    std::memcpy(packetData, &opcode, sizeof(int));

    size_t sz=strlen(FILENAME);
    std::memcpy(packetData+4,FILENAME,sz);

	Packet* packet=new Packet(cur_ack_no,packetData,sz);
    
	return packet;
}

/*
    	----------------------
WR  	| 02   |   FileName  |
		----------------------

*/

Packet* build_write_packet()
{
    memset(packetData,'\0',BUFFER_SIZE);

    int opcode = htonl(2);
    // Pack Opcode (Big-Endian)
    std::memcpy(packetData, &opcode, sizeof(int));

	size_t sz=strlen(FILENAME);
    std::memcpy(packetData+4,FILENAME,sz);

	Packet *packet=new Packet(cur_ack_no,packetData,sz);
	return packet;
}

/*
    	---------------------
ACK  	| 03  |  next_ack#  |
		----------------------

*/

Packet* build_ack_packet(int next_ack_no)
{
    memset(packetData,'\0',BUFFER_SIZE);

    int opcode = htonl(3);
    // Pack Opcode (Big-Endian)
    std::memcpy(packetData, &opcode, sizeof(int));

    next_ack_no=htonl(next_ack_no);

    std::memcpy(packetData+4,&next_ack_no,sizeof(int));

	Packet *packet=new Packet(cur_ack_no,packetData,4);
	return packet;
}

/*
    	-----------------------
NACK 	| 04   |   Req_Ack #  |
		-----------------------

*/

Packet* build_nack_packet(int req_ack_no)
{
    memset(packetData,'\0',BUFFER_SIZE);

    int opcode = htonl(4);
    // Pack Opcode (Big-Endian)
    std::memcpy(packetData, &opcode, sizeof(int));

    req_ack_no=htonl(req_ack_no);

    std::memcpy(packetData+4,&req_ack_no,sizeof(int));

	Packet* packet=new Packet(cur_ack_no,packetData,4);
	return packet;
}


/*
    	-------------------------------------------------------
DATA  	| 05   |   ACK #  |  bytes Sent | Data (<=512 bytes)  |
		-------------------------------------------------------

*/

pair<Packet*,size_t> build_data_packet()
{
	//read 512(atmost) for the current file starting at cur_block_ptr

    memset(packetData,'\0',BUFFER_SIZE);

    int opcode = htonl(5);
    // Pack Opcode (Big-Endian)
    std::memcpy(packetData, &opcode, sizeof(int));

    // Pack ACK number (Big-Endian)
    int ack=htonl(cur_ack_no);
    std::memcpy(packetData + 4, &ack, sizeof(int));

    // Read file data (at most 512 bytes)
    int bytesRead = readFromFile(packetData + 12, 512);
    bytesRead=htonl(bytesRead);
    //pack the bytesRead
    std::memcpy(packetData+8,&bytesRead,sizeof(int));

	Packet *packet=new Packet(cur_ack_no,packetData,bytesRead+4);
	return {packet,bytesRead};
}

/*
    	------------------
ERR  	| 06  |  Message |
		------------------

*/

Packet* build_err_packet()
{
    memset(packetData,'\0',BUFFER_SIZE);

    int opcode = htonl(6);
    // Pack Opcode (Big-Endian)
    std::memcpy(packetData, &opcode, sizeof(int));

    const char* message="Error occurred. Terminating transmission.";
    std::memcpy(packetData+4,message,(size_t)(strlen(message)));

	Packet *packet=new Packet(cur_ack_no,packetData,strlen(message));
	return packet;
}
