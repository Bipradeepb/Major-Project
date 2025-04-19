#include "headers/globals.hpp"

using namespace std;

u_char readbuffer[BUFFER_SIZE];

void processData(Context *ctx)
{
    //extract data and create jobs
    cout<<"Beginning processing of data with ";
    int opcode;
    std::memcpy(&opcode, readbuffer, sizeof(opcode));  // Extract opcode
    opcode=ntohl(opcode); //change endianness

    cout<<"opcode="<<opcode<<"\n";

    switch (opcode) {

        case 1: {

            //RD packet
            size_t max_filename_length = BUFFER_SIZE - 4; // Max possible length after opcode
            size_t filename_length = strnlen(reinterpret_cast<const char*>(readbuffer + 4), max_filename_length);

             if (filename_length == 0) {
                std::cout << "Error: Empty filename! Discarding packet." << std::endl;

                break;
            }


            // Copy the filename safely
            char filename[max_filename_length + 1] = {}; 
            std::memcpy(filename, readbuffer + 4, filename_length);
            filename[filename_length] = '\0'; // Ensure null termination

            // Create Job
            Job job(1, filename, filename_length);

            //push to jobQueue
            {
                mtx_jobQueue.lock();
                ctx->jobQueue.push_back(job);
                mtx_jobQueue.unlock();
            }

            cv_jobQueue.notify_one();

            std::cout << "Opcode 1: Received a RD packet with Filename = " << filename << std::endl;
            LOG("Opcode 1: Received a RD packet with Filename = ",std::string(filename));
            break;
        }
        case 2:{

            //WR packet
            size_t max_filename_length = BUFFER_SIZE - 4; // Max possible length after opcode
            size_t filename_length = strnlen(reinterpret_cast<const char*>(readbuffer + 4), max_filename_length);

            // Copy the filename safely
            char filename[max_filename_length + 1] = {}; // +1 for null terminator
            std::memcpy(filename, readbuffer + 4, filename_length);
            filename[filename_length] = '\0'; // Ensure null termination

            // Create Job
            Job job(2, filename, filename_length);

            //push to jobQueue
            {
                mtx_jobQueue.lock();
                ctx->jobQueue.push_back(job);
                mtx_jobQueue.unlock();
            }

            cv_jobQueue.notify_one();

            std::cout << "Opcode 2: Received a WR packet with Filename = " << filename << std::endl;
            LOG("Opcode 2: Received a WR packet with Filename = ",std::string(filename));
            break;
        }
        case 3:  {

            //extract the next sequence number
            int next_ack;
            std::memcpy(&next_ack,readbuffer+4,sizeof(next_ack));
            next_ack=ntohl(next_ack);

            // Prepare the data buffer (only for next_ack)
            char data[sizeof(next_ack)]; // Exact size needed for packing

            // Copy the ack into the data buffer
            std::memcpy(data, &next_ack, sizeof(next_ack));  // No network byte order conversion

            // Create the job
            Job job(3, data, sizeof(next_ack));

            //push to jobQueue
            {
                mtx_jobQueue.lock();
                ctx->jobQueue.push_back(job);
                mtx_jobQueue.unlock();
            }

            cv_jobQueue.notify_one();

            std::cout << "Opcode 3: Received an ACK packet with ackno = " << next_ack << std::endl;
            LOG("Opcode 3: Received an ACK packet with ackno = ",to_string(next_ack));
            break;

        }
        case 4: {

            //extract the sequence number to be resent
            int req_ack;
            std::memcpy(&req_ack,readbuffer+4,sizeof(req_ack));
            req_ack=ntohl(req_ack);

            // Prepare the data buffer (only for next_ack)
            char data[sizeof(req_ack)]; // Exact size needed for packing

            // Copy the ack into the data buffer
            std::memcpy(data, &req_ack, sizeof(req_ack));  // No network byte order conversion

            // Create the job
            Job job(4, data, sizeof(req_ack));

            //push to jobQueue
            {
                mtx_jobQueue.lock();
                ctx->jobQueue.push_back(job);
                mtx_jobQueue.unlock();
            }

            cv_jobQueue.notify_one();

            std::cout << "Opcode 4: Received a NACK packet with ackno = " << req_ack << std::endl;
            LOG("Opcode 4: Received a NACK packet with ackno = ",to_string(req_ack));
            break;

        }
        case 5:{

            //Data packet

            // Extract cur_ack (next 4 bytes after opcode)
            int cur_ack;
            std::memcpy(&cur_ack, readbuffer + 4, sizeof(cur_ack));
            cur_ack = ntohl(cur_ack);  // Convert from network to host byte order

            int receivedLen;
            std::memcpy(&receivedLen,readbuffer+8,sizeof(receivedLen));

            //calculate the size of data sent
            int actual_data_length=ntohl(receivedLen);

            size_t total_data_size=actual_data_length+8;

            // Prepare the final data buffer
            char data[total_data_size];

            // Pack cur_ack into the data buffer (first 4 bytes)
            std::memcpy(data, &cur_ack, sizeof(cur_ack));

            //Pack actual data length into the data buffer (next 4bytes)
            std::memcpy(data+4,&actual_data_length,sizeof(int));

            // Copy remaining data (only actual length)
            std::memcpy(data + 8, readbuffer + 12, actual_data_length);


            // Create the job
            
            // Verify if data is less than 512 bytes
            if (actual_data_length < 512) {
                
                Job job(6, data, total_data_size);

                // Push job to queue safely
                {
                    mtx_jobQueue.lock();
                    ctx->jobQueue.push_back(job);
                    mtx_jobQueue.unlock();
                }

                cv_jobQueue.notify_one();
            }
            else{

                Job job(5, data, total_data_size);

                // Push job to queue safely
                {
                    mtx_jobQueue.lock();
                    ctx->jobQueue.push_back(job);
                    mtx_jobQueue.unlock();
                }

                cv_jobQueue.notify_one();
            }

            std::cout << "Opcode 5: Received a DATA packet with ackno = " << cur_ack << std::endl;
            LOG("Opcode 5: Received a DATA packet with ackno = ",to_string(cur_ack));
            break;

        }
        case 6: {

            //ERR packet
            size_t max_message_length = BUFFER_SIZE - 4; // Max possible length after opcode
            size_t message_length = strnlen(reinterpret_cast<const char*>(readbuffer + 4), max_message_length);

             if (message_length == 0) {
                std::cout << "Error: Empty filename! Discarding packet." << std::endl;

                break;
            }

            // Copy the filename safely
            char message[max_message_length + 1] = {}; // +1 for null terminator
            std::memcpy(message, readbuffer + 4, message_length);
            message[message_length] = '\0'; // Ensure null termination

            // Create Job
            Job job(7, message, message_length);

            //push to jobQueue
            {
                mtx_jobQueue.lock();
                ctx->jobQueue.push_back(job);
                mtx_jobQueue.unlock();
            }

            cv_jobQueue.notify_one();


            std::cout << "Opcode 7: Received a ERR packet with message = " << message << std::endl;
            LOG("Opcode 7: Received a ERR packet with message = ",std::string(message));
            break;
        }
        default:
            std::cout << "Invalid Opcode: " << opcode << std::endl;
    }

}

void reader_thread(int sockfd,Context *ctx)
{
    cout<<"Started Reader Thread..."<<"\n";

    //reader thread has started

    //setup
    struct sockaddr_in client_addr;
    socklen_t client_len;
    ssize_t received_bytes;

    while(1)
    {
        //keep listening to incoming packets
        // read from socket()
        bzero(readbuffer,BUFFER_SIZE);

        // Set up the file descriptor set
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);

        // Set up the timeout structure
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        // Use select to wait for the socket to be ready for reading
        int select_result = select(sockfd + 1, &readfds, NULL, NULL, &timeout); 
        //prevent blocking recvfrom call

        if (select_result < 0) {
            // An error occurred with select
            perror("select error");
            continue;
        } else if (select_result == 0) {
            // Timeout occurred, no data to read
            // printf("recvfrom timed out\n");
            this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }

        memset(readbuffer,'\0',sizeof(readbuffer));

        client_len = sizeof(client_addr);
        received_bytes = recvfrom(sockfd, readbuffer, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &client_len);            

        check_err(received_bytes,"recvfrom failed");

        readbuffer[received_bytes] = '\0';  // Null-terminate the received data

        char incoming_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_addr.sin_addr), incoming_ip, INET_ADDRSTRLEN);

        int opcode;
        std::memcpy(&opcode, readbuffer, sizeof(opcode));  // Extract opcode
        opcode=ntohl(opcode); //change endianness

        // If client_ip is not set, initialize it
        if (!ctx->is_client_ip_set) {

            if(opcode==1||opcode==2)
            {
                //only if RD/WR packet
                strcpy(ctx->clientIp, incoming_ip);
                ctx->clientPort = ntohs(client_addr.sin_port);
                ctx->is_client_ip_set=true;
                cout << "Client IP set to: " <<ctx->clientIp<< "\n";
                LOG("Client IP set to: ",std::string(ctx->clientIp));
            }
            else
            {
                continue;
            }
        } 
        // If IP doesn't match, discard the packet
        else if (strcmp(ctx->clientIp, incoming_ip) != 0) {

            //only one client supported at a time
            cout << "Packet dropped from unauthorized IP: " << incoming_ip << "\n";
            LOG("Packet dropped from unauthorized IP: ",std::string(incoming_ip));
            continue;
        }
        

        //data always present--> need to process the data now
        processData(ctx);
    }
}