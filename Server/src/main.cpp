
// how to use -->prithi@HP$./server.out <ServerPort>
#include "globals.hpp"
#include "packets.hpp"
#include "Logger.hpp"

/*

Header Formats

   Type   Op #     Format without header

          2 bytes    string   1 byte    2 byte
          ------------------------------------------
   RRQ/  | 01/02 |  Filename  |   0  |  WindowSize |
   WRQ    ------------------------------------------
          2 bytes    2 bytes       n bytes
          ---------------------------------
   DATA  | 03    |   Block #  |    Data    |
          ---------------------------------
          2 bytes    2 bytes
          -------------------
   ACK   | 04    |   Block #  |
          --------------------

Note1:    The data field is from zero to 512 bytes long.  If it is 512 bytes
   	  long, the block is not the last block of data; if it is from zero to
   	  511 bytes long, it signals the end of the transfer.

Note2:  a.A WRQ is acknowledged with an ACK packet having a block number of zero.
	b.The WRQ and DATA packets are acknowledged by ACK or ERROR packets
	c.The RRQ and ACK packets are acknowledged by  DATA  or ERROR packets.
	d. All  packets other than duplicate ACK's and those used for termination are acknowledged unless a timeout occurs

*/

void serverAsWriter(int sockfd,Context *ctx);
void serverAsReader(int sockfd,Context *ctx);

const char* SHARED_MEM_NAME = "/context_shared_mem"; // Shared memory secret key
#define SEM_NAME "/my_binary_semaphore"

int main(int argc, char **argv){

	// checking command line args
	if (argc <2){
		printf("Usage To Enable Verbose Logging to file$	LOG_ON_FILE=1 ./build/ser_exe <ServerPort> V\n");
		printf("Usage To Enable Verbose Logging to Terminal$	./build/ser_exe <ServerPort> V\n");
		printf("Usage To Min Logging to Terminal$	./build/ser_exe <ServerPort>\n");
		exit(1);
	}
	if(argc ==3){// checking for verbose logging
		if(std::string(argv[2])== "V"){
			LOG_Verbose = true;
		}
	}
	else if(argc == 2){
		LOG_Verbose = false;
	}

    // Try to create the semaphore, if already exists just open it
    sem_t* sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0666, 0);
    if (sem == SEM_FAILED) {
        if (errno == EEXIST) {
            // Semaphore already exists, just open it
            sem = sem_open(SEM_NAME, 0);
        } else {
            perror("sem_open failed");
            return 1;
        }
    }

	//creating the socket
	LOG_TO(LogDestination::TERMINAL_ONLY,"Socket Setup\n");
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

	//////////////////////////////////// Setting UP Context for ACTIVE / BACKUP /////////////////
    int shm_fd;
    bool is_new = false;

    // Try opening existing shared memory
    shm_fd = shm_open(SHARED_MEM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        // If it doesn't exist, create it
        shm_fd = shm_open(SHARED_MEM_NAME, O_CREAT | O_RDWR, 0666);
        if (shm_fd == -1) {
            perror("shm_open failed");
            return 1;
        }
        is_new = true;
        LOG_TO(LogDestination::TERMINAL_ONLY, "Created new shared memory.\n");
    } else {
        LOG_TO(LogDestination::TERMINAL_ONLY, "Using existing shared memory.\n");
    }

    // Resize only if it's newly created
    if (is_new && ftruncate(shm_fd, sizeof(Context)) == -1) {
        perror("ftruncate failed");
        return 1;
    }

    // Map shared memory to process address space
    Context* ctx = (Context*)mmap(0, sizeof(Context), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ctx == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }

setup:
    // If new, initialize memory and Fill up Context For 1st Time
    if (is_new) {
        memset(ctx, 0, sizeof(Context));

		/////////////////////////////////// SyncIng With Client //////////////////
		struct sockaddr_in clientaddr{};
		socklen_t addr_len = sizeof(clientaddr);

		u_char buffer[BUFFER_SIZE]={0};

		// Receive RD/WR packet
		LOG_TO(LogDestination::BOTH,"Waiting For RD/WR of Client\n");
		ssize_t recv_len = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&clientaddr, &addr_len);
		check_err(recv_len,"RD/WR recv failed");

		// Extract RRQ/WRQ data
		RRQ_WRQ_Packet pkt = extract_rrq_wrq_packet(buffer);

		// Fill shared memory Context ctx
		inet_ntop(AF_INET, &clientaddr.sin_addr, ctx->clientIp, sizeof(ctx->clientIp));
		ctx->clientPort = ntohs(clientaddr.sin_port);
		ctx->WindowSize = pkt.WinSize;
		ctx->current_blk = pkt.block_number;
		strncpy(ctx->fileName, pkt.filename, sizeof(ctx->fileName) - 1);
		ctx->fileName[sizeof(ctx->fileName) - 1] = '\0';  // Ensure null termination
		ctx->curr_Win = ctx->WindowSize;

		LOG_TO(LogDestination::BOTH, "Client IP: " ,ctx->clientIp ,"\n");
		LOG_TO(LogDestination::BOTH, "Client Port: " ,ctx->clientPort ,"\n");
		LOG_TO(LogDestination::BOTH, "Server Window Size: " ,ctx->WindowSize ,"\n");
		LOG_TO(LogDestination::BOTH, "Current_blk: " ,ctx->current_blk ,"\n");
		LOG_TO(LogDestination::BOTH, "File Path To Be Read/Written: " ,ctx->fileName ,"\n------------------------------------------------------------------\n");

		if(buffer[1]==1){ // Read Packet recv
			ctx->choice='R';
			LOG_TO(LogDestination::BOTH,"Recv Read RQ\n");
			LOG_TO(LogDestination::TERMINAL_ONLY,"Ongoing FileTransfer...\n\n");
			serverAsWriter(sockfd,ctx);
		}
		else{ // Write Packet recv
			ctx->choice='W';
			LOG_TO(LogDestination::BOTH,"Recv Write RQ\n");
			LOG_TO(LogDestination::TERMINAL_ONLY,"Ongoing FileTransfer...\n\n");
			serverAsReader(sockfd,ctx);
		}
	}
	else{// This process is a Backup

		LOG_TO(LogDestination::BOTH,"Waiting On Semaphore\n");
		sem_wait(sem);  // Block until producer[switch] signals

		if(ctx->choice == 'R'){
			LOG_TO(LogDestination::BOTH,"Continuing Read RQ\n");
			LOG_TO(LogDestination::TERMINAL_ONLY,"Ongoing FileTransfer...\n\n");
			serverAsWriter(sockfd,ctx);
		}

		else{
			LOG_TO(LogDestination::BOTH,"Continuing Write RQ\n");
			LOG_TO(LogDestination::TERMINAL_ONLY,"Ongoing FileTransfer...\n\n");
			serverAsReader(sockfd,ctx);
		}

	}


	// Reach here => Single File Transfer complete
	LOG_TO(LogDestination::TERMINAL_ONLY,"File Transfer Complete\n\n");
	LOG_TO(LogDestination::BOTH,"Setting up server for Next File Transfer\n");
	is_new = true; // setting up for Next File Transfer
	goto setup;

	return 0;
}
