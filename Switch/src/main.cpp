/*
This is a switching emulator cum Watchdog for FTP service

Sits between Client Layer and server Layer
*/
#include "Globals.hpp"

void reader_thread(int sockfd);
void frwd_thread(int sockfd);
void accept_thread_func();
void load_configuration(const std::string& fileName);

int main(int argc, char **argv){

	// checking command line args
	if (argc <2){
        std::cout << "Contents of Switch Config File :-\n";
        std::cout << "The first Line has Threshold- 2*WindowSize+1\n";
        std::cout << "The second Line has Active Server IP_PORT\n";
        std::cout << "The third Line has Back Server IP_PORT\n";
        std::cout << "The fourth line has  Switch Port\n";
        printf("\nUsage $:- ./build/sw_exe ./config.txt\n");
        exit(1);
	}

	//init global data-structures
	load_configuration(argv[1]);

	//creating the UDP socket
	int sockfd= socket(AF_INET,SOCK_DGRAM,0);
	check_err(sockfd,"Error in opening UDP socket");

	// setting server details
	struct sockaddr_in sa;
	bzero(&sa,sizeof(sa));
	sa.sin_family = AF_INET;//setting address family to IPv4
	sa.sin_port = htons(sw_port);
	sa.sin_addr.s_addr=INADDR_ANY;

	// binding
	int status= bind(sockfd,(struct sockaddr *) &sa, sizeof(sa));
	check_err(status,"Error in binding");

	//optional messages
	printf("\nSetup Finished Sarting 2 threads ...\n\n");

    //////////// Setting Up TCP socket ..........
    struct sockaddr_in address;

    // Create server socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Bind
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(sw_port);
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    listen(server_fd, 3);
    /////////////////////////////// Tcp setup done

    //create thread
	std::thread reader(reader_thread,sockfd);
	std::thread frwd(frwd_thread,sockfd);
    std::thread accept_thread(accept_thread_func);    // Start accept thread [Accept New Tcp clients(backp server)]

    // Signal accept_thread to accept a New connection
    {
        std::lock_guard<std::mutex> lock(mtx);
        allow_next_accept = true;
    }
    cv.notify_one();

    //join thread
	reader.join();
	frwd.join();
    accept_thread.join();

    close(server_fd);//closing tcp socket
    close(sockfd);//closing udp socket

	return 0;
}


void accept_thread_func() {
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);

    while (true) {
        // Wait until main thread signals it's ready for next connection
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [] { return allow_next_accept; });
        }

        // Accept new connection
        int new_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addrlen);
        if (new_socket < 0) {
            perror("Accept failed");
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(mtx);
            client_socket = new_socket;
            allow_next_accept = false;
        }

    }
}


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

	// Read the last line as Switch port
    std::string lastLine;
    if (!std::getline(file, lastLine)) {
        throw std::runtime_error("Failed to read switch port from file.");
    }

    try {
        sw_port = std::stoi(lastLine);
    } catch (const std::invalid_argument& e) {
        throw std::runtime_error("Switch port is not a valid integer.");
    }

    file.close();
}
