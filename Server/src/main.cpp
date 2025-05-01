/*

Header Formats (Similar to TFTP)

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

#include "globals.hpp"
#include "packets.hpp"
#include "Logger.hpp"


struct Config {
    int server_port;
    std::string switch_ip;
    int switch_port;
};

bool readConfigFromFile(const std::string& filename, Config& config);
void serverAsWriter(int sockfd,Context *ctx);
void serverAsReader(int sockfd,Context *ctx);


const char* SHARED_MEM_NAME = "/context_shared_mem"; // Shared memory secret key

int main(int argc, char **argv){

	// checking command line args
	if (argc <2){
		std::cout << "Contents of Server Config File :-\n";
        std::cout << "The first Line has Server Port\n";
        std::cout << "The second Line has Switch Ip\n";
        std::cout << "The third Line has Switch Port\n";
		std::cout << "The fourth Line has Timeout in ms\n";
		printf("\nUsage To Enable Verbose Logging to file$	LOG_ON_FILE=1 ./build/ser_exe <config.txt> V\n");
		printf("Usage To Enable Verbose Logging to Terminal$	./build/ser_exe <config.txt> V\n");
		printf("Usage To Min Logging to Terminal$	./build/ser_exe <config.txt>\n");
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

	//Loading Config:-
	Config theConfig;
	readConfigFromFile(argv[1],theConfig);

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

////////////////////////////////// Each iteration does 1 complete file transfer
while(1){
	/////////////////////////////////////////creating the UDP socket
	LOG_TO(LogDestination::TERMINAL_ONLY,"Socket Setup\n");
	int sockfd= socket(AF_INET,SOCK_DGRAM,0);
	check_err(sockfd,"Error in opening UDP socket");

	// setting server details
	struct sockaddr_in sa;
	bzero(&sa,sizeof(sa));
	sa.sin_family = AF_INET;//setting address family to IPv4
	sa.sin_port = htons(theConfig.server_port);
	sa.sin_addr.s_addr=INADDR_ANY;

	// binding
	int status= bind(sockfd,(struct sockaddr *) &sa, sizeof(sa));
	check_err(status,"Error in binding");
	/////////////////////////////////////////////////////////////

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

		if(buffer[1]!=1 and buffer[1]!=2){
			std::cerr << "Bad packet Recv \n";
			return 1;//Quitting
		}

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
	} // end of Active setup code

	else{// This process is a Backup

		LOG_TO(LogDestination::BOTH,"Waiting For Switch Signal\n");

		/// Setting up TCP socket and waiting for signal(mssg) from Switch
			int sock = 0;
			struct sockaddr_in serv_addr;
			char buffer[1024] = {0};

			// Create TCP socket
			sock = socket(AF_INET, SOCK_STREAM, 0);
			if (sock < 0) {
				perror("TCP Socket creation error");
				return -1;
			}

			serv_addr.sin_family = AF_INET;
			serv_addr.sin_port = htons(theConfig.switch_port);

			// Convert IPv4 address from text to binary form
			if (inet_pton(AF_INET, theConfig.switch_ip.c_str(), &serv_addr.sin_addr) <= 0) {
				perror("Invalid Switch address");
				return -1;
			}

			// Connected to Switch
			if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
				perror("TCP Connection Failed");
				return -1;
			}

        memset(buffer, 0, sizeof(buffer));
        read(sock, buffer, 1024); // read from switch [Blocking Call]
		close(sock);

        std::string received(buffer);
		if (received == "switch") {

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

	}// end of backup code

	// Reach here => Single File Transfer complete
	LOG_TO(LogDestination::TERMINAL_ONLY,"File Transfer Complete\n\n");
	LOG_TO(LogDestination::BOTH,"Setting up server for Next File Transfer\n");
	is_new = true; // setting up for Next File Transfer
	close(sockfd);

}// end of while inf

	return 0;
}

bool readConfigFromFile(const std::string& filename, Config& config) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return false;
    }

    std::string line;

    // Read server port
    if (std::getline(file, line)) {
        config.server_port = std::stoi(line);
    } else {
        std::cerr << "Error reading server port." << std::endl;
        return false;
    }

    // Read switch IP
    if (std::getline(file, line)) {
        config.switch_ip = line;
    } else {
        std::cerr << "Error reading switch IP." << std::endl;
        return false;
    }

    // Read switch port
    if (std::getline(file, line)) {
        config.switch_port = std::stoi(line);
    } else {
        std::cerr << "Error reading switch port." << std::endl;
        return false;
    }

	//Read Timeout
    if (std::getline(file, line)) {
        TIMEOUT_MILLI_SEC = std::stoi(line);
    } else {
        std::cerr << "Error reading Timeout(ms)." << std::endl;
        return false;
    }

    file.close();
    return true;
}
