/*
This is a switching emulator cum Watchdog for FTP service

Sits between Client Layer and server Layer
*/
#include "Globals.hpp"

void reader_thread(int sockfd);
void frwd_thread(int sockfd);

void load_configuration(const std::string& fileName) {
    std::ifstream file(fileName);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + fileName);
    }

    // Read threshold
    if (!(file >> threshold)) {
        throw std::runtime_error("Failed to read threshold from file.");
    }

    // Consume the remaining newline character after reading the integer
    file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    // Read the first server_list element
    if (!std::getline(file, server_list[0])) {
        throw std::runtime_error("Failed to read first server_list entry from file.");
    }

    // Read the second server_list element
    if (!std::getline(file, server_list[1])) {
        throw std::runtime_error("Failed to read second server_list entry from file.");
    }

    file.close();
}

#define SEM_NAME "/my_binary_semaphore"
#define SHM_PATH "/dev/shm/context_shared_mem"
static int sockfd; // limiting to this translational unit

void signalHandler(int signal) {
    std::cout << "\nReceived signal: " << signal << " (SIGINT). Exiting gracefully...\n";

    // closing socket
    close(sockfd);
    // Remove the named semaphore
    if (sem_unlink(SEM_NAME) == 0) {
        std::cout << "Semaphore " << SEM_NAME << " removed successfully.\n";
    } else {
        perror("sem_unlink failed");
    }
    // Remove the shared memory file
    if (std::system(("rm -f " + std::string(SHM_PATH)).c_str()) == 0) {
        std::cout << "Shared memory " << SHM_PATH << " removed successfully.\n";
    } else {
        std::cerr << "Failed to remove shared memory " << SHM_PATH << "\n";
    }

    exit(1);  // Terminate the program
}

int main(int argc, char **argv){

	// checking command line args
	if (argc <2){
		printf("Enter format :- ./build/sw_exe <sw_Port> <configFilePath>\n");
		std::cout<<"Config File Format :\nThe first line contains threshold (an integer).\nThe second line contains the active server(Ip_Port).\nThe third line contains the backup server(Ip_Port).\n";
		exit(1);
	}

    //handle graceful termination
    std::signal(SIGINT, signalHandler);

	//init global data-structures
	load_configuration(argv[2]);
	
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
	printf("\nSetup Finished Sarting 2 threads ...\n\n");

	//Create Semaphore and Init to 0 if NOt exist else Use Existing
	sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0666, 0);
    if (sem == SEM_FAILED) {
        if (errno == EEXIST) {
            // Semaphore already exists, just open it
            sem = sem_open(SEM_NAME, 0);
        } else {
            perror("sem_open failed");
            return 1;
        }
    }

    //create thread	
	std::thread reader(reader_thread,sockfd);
	std::thread frwd(frwd_thread,sockfd);

    //join thread
	reader.join();
	frwd.join();
	
	return 0;
}