
// how to use -->prithi@HP$./server.out <ServerPort>
#include "globals.hpp"

/*

Order of Headers

    2 bytes [Application layer Protocol]
    ----------------------------------------------------------
   |  Local Medium  |  Internet  |  Datagram  |  TFTP Opcode  |
    ----------------------------------------------------------

std-TFTP Formats

   Type   Op #     Format without header

          2 bytes    string   1 byte     string   1 byte
          -----------------------------------------------
   RRQ/  | 01/02 |  Filename  |   0  |    Mode    |   0  |
   WRQ    -----------------------------------------------
          2 bytes    2 bytes       n bytes
          ---------------------------------
   DATA  | 03    |   Block #  |    Data    |
          ---------------------------------
          2 bytes    2 bytes
          -------------------
   ACK   | 04    |   Block #  |
          --------------------
          2 bytes  2 bytes        string    1 byte
          ----------------------------------------
   ERROR | 05    |  ErrorCode |   ErrMsg   |   0  |
   	  ----------------------------------------

Note1:    The data field is from zero to 512 bytes long.  If it is 512 bytes
   	  long, the block is not the last block of data; if it is from zero to
   	  511 bytes long, it signals the end of the transfer.
   
Note2:  a.A WRQ is acknowledged with an ACK packet having a block number of zero.
	b.The WRQ and DATA packets are acknowledged by ACK or ERROR packets
	c.The RRQ and ACK packets are acknowledged by  DATA  or ERROR packets.
	d. All  packets other than duplicate ACK's and those used for termination are acknowledged unless a timeout occurs	

Note3:  The packets sent from this server is prepended by client_id| 1b \0 |std-tftp packet
		The packets recv by this server is prepended by client_id | 1b\0 | fileName | \0 | std-tftp packet

*/

void reader_thread(int sockfd);
void frwd_thread(int sockfd);
void timer_thread();


int main(int argc, char **argv){

	// checking command line args
	if (argc <2){
		printf("Enter format :- ./build/server_exe <server_Port>\n");
		std::cout<<"Suggestion:- Give 9000s port to Layer-3(Tftp servers) \n Give 8000s port to Layer-2(LoadBalencers) \nGive 7000s port to Layer-1(sw_emulator)\n";
		exit(1);
	}
	
	//creating the socket
	int sockfd= socket(AF_INET,SOCK_DGRAM,0);
	check_err(sockfd,"Error in opening UDP socket");
	
	// setting server details
	struct sockaddr_in sa;
	bzero(&sa,sizeof(sa));
	sa.sin_family = AF_INET;//setting address family to IPv4
	sa.sin_port = htons(atoi(argv[1]));
	sa.sin_addr.s_addr=INADDR_ANY;
	
	// binding
	int status= bind(sockfd,(struct sockaddr *) &sa, sizeof(sa));
	check_err(status,"Error in binding");
	
	//optional messages
	printf("\nSetup Finished Sarting 3 threads ...\n\n");
			
	std::thread reader(reader_thread,sockfd);
	std::thread frwd(frwd_thread,sockfd);
	std::thread timer(timer_thread);

	reader.join();
	frwd.join();
	timer.join();

	
	return 0;
}













