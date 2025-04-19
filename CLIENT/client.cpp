#include "headers/globals.hpp"
#include "headers/helper_functions.cpp"
#include "headers/handle_packets.cpp"
#include "read.cpp"
#include "send.cpp"
#include "work.cpp"

using namespace std;

int sockfd;

void send_read_packet()
{
	//create a RD packet
    Packet *packet=build_read_packet();
    ssize_t sent_bytes = sendto(sockfd, packet->data, BUFFER_SIZE, 0,
                            (struct sockaddr*)&server_addr, sizeof(server_addr));

    if(sent_bytes<0)
    {
        perror("Send to failed.\n");
        exit(1);
    }
}

void send_write_packet()
{
    //create a WR packet
    Packet *packet=build_write_packet();

    ssize_t sent_bytes = sendto(sockfd, packet->data, BUFFER_SIZE, 0,
                            (struct sockaddr*)&server_addr, sizeof(server_addr));

    if(sent_bytes<0)
    {
        perror("Send to failed.\n");
        exit(1);
    }
}

int main(int argc, char **argv) {
    if (argc < 4) {
        printf("./client_exe <ServerIP> <ServerPort> <choice>\n");
        printf("choice = r (read to server) / w (write to server)\n");
        printf("ServerIP and Port is that of the Switch\n");
        exit(1);
    }

    

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // Define server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2])); // Fixing the incorrect index

    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0) {
        perror("Invalid address / Address not supported");
        return 1;
    }

    // Convert and print the server address
    char ip_str[INET_ADDRSTRLEN];  // Buffer for IP string
    if (inet_ntop(AF_INET, &server_addr.sin_addr, ip_str, sizeof(ip_str)) == nullptr) {
        perror("inet_ntop failed");
        return 1;
    }

    std::cout << "Server Address: " << ip_str << ":" << ntohs(server_addr.sin_port) << std::endl;

    //socket creation completed
    //reset all the data structures before start
    resetDS();

    //Ask user for filename
    cout<<"Enter FILENAME"<<endl;

    if (fgets(FILENAME, 200, stdin) != NULL) {
        // Remove the trailing newline character if present
        size_t len = strlen(FILENAME);
        if (len > 0 && FILENAME[len - 1] == '\n') {
            FILENAME[len - 1] = '\0';
        }
    } else {
        cout<<"Error reading input"<<endl;
        free(FILENAME);
        exit(1);
    }

    if(argv[3][0]=='r')
    {
        send_read_packet();
        mode=1; //receiver mode
    }
    else if(argv[3][0]=='w')
    {
        send_write_packet();
        mode=0; //sender mode

        mtx_window.lock();
        {
            //create window of packets
            for(int i=1;i<=windowSize;i++)
            {
                auto pr=build_data_packet();
                size_t dataSize=pr.second;
                Packet *packet=pr.first;
                window.push_back(packet);

                //check if last block has been read
                if(cur_block_ptr>=getFileSize())
                {
                    //if the data in the packet is less than 512 
                    if(dataSize<512)
                    {
                        isTransmissionON=false;
                        break;
                    }

                    //if the data in the packet is equal to 512
                    if(dataSize==512&&i==windowSize)
                    {
                        break;
                    }
                    //if it is the last packet of this window,keep transmission ON
                    //transmit a NULL Packet 
                    isTransmissionON=false;
                    auto pr1=build_data_packet();
                    Packet *nullPacket=pr1.first;
                    window.push_back(nullPacket);

                    break;
                }

                //update cur_ack_no
                cur_ack_no++;
            }

            cur_ack_no=startSEQN;
        }
        mtx_window.unlock();

        {
            std::unique_lock<std::mutex> lock(mtx);

            if(should_sleep)
            {
                should_sleep=false;
                cv.notify_all();
            }
            else
            {
                cv.wait(lock,[]{return should_sleep;});
                should_sleep=false;
                cv.notify_all();
            }
        }
    }
    else
    {
        cout<<"Invalid input.\n";
        exit(1);
    }

    printf("\nSetup Finished Sarting 3 threads ...\n\n");
            
    std::thread reader(reader_thread,sockfd);
    std::thread sender(sender_thread,sockfd);
    std::thread worker(worker_thread,sockfd);

    worker.join();
    //worker always terminates transmission
    stop_thread=true;

    cout<<"Transmission completed successfully. Waiting for graceful shutdown."<<endl;
    reader.join();
    sender.join();


    close(sockfd); // Closing socket before exiting
    cout<<"Sockets closed. Process completed successfully"<<endl;
    return 0;
}
