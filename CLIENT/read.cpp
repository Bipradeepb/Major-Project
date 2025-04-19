#include "headers/globals.hpp"

using namespace std;

u_char readbuffer[BUFFER_SIZE];

void processData()
{
    //extract data and create jobs
    int opcode;
    std::memcpy(&opcode, readbuffer, sizeof(opcode));  // Extract opcode
    opcode=ntohl(opcode); //change endianness

    switch (opcode) {

        case 1: {

            //RD packet
            //client never receives RD packet so this case is never encountered
            break;
        }
        case 2:{

            //WR packet
            //client never receives WR packet so this case is also never encountered
        }
        case 3:  {

            //client receives ACK when it is in write mode

            //extract the next sequence number
            int next_ack;
            std::memcpy(&next_ack,readbuffer+4,sizeof(next_ack));
            next_ack=ntohl(next_ack);

            // Prepare the data buffer (only for next_ack)
            char data[sizeof(next_ack)]; // Exact size needed for packing

            // Copy the ack into the data buffer
            std::memcpy(data, &next_ack, sizeof(next_ack));  // No network byte order conversion

            // Create the job
            Job* job=new Job(3, data, sizeof(next_ack));

            //push to jobQueue
            {
                mtx_jobQueue.lock();
                jobQueue.push(job);
                mtx_jobQueue.unlock();

                cv_jobQueue.notify_one();
            }

            std::cout << "Opcode 3: Received an ACK packet with ackno = " << next_ack << "\n";
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
            Job* job=new Job(4, data, sizeof(req_ack));

            //push to jobQueue
            {
                mtx_jobQueue.lock();
                jobQueue.push(job);
                mtx_jobQueue.unlock();

                cv_jobQueue.notify_one();
            }

            std::cout << "Opcode 4: Received a NACK packet with ackno = " << req_ack << "\n";
            break;

        }
        case 5:{

            //Data packet

            // Extract cur_ack (next 4 bytes after opcode)
            int cur_ack;
            std::memcpy(&cur_ack, readbuffer + 4, sizeof(cur_ack));
            cur_ack = ntohl(cur_ack);  // Convert from network to host byte order

            //Extract the bytes sent
            int receivedLen;
            std::memcpy(&receivedLen,readbuffer+8,sizeof(int));

            int actual_data_length=ntohl(receivedLen);

            size_t total_data_size=actual_data_length+8;

            // Prepare the final data buffer
            char data[total_data_size];

            // Pack cur_ack into the data buffer (first 4 bytes)
            std::memcpy(data, &cur_ack, sizeof(cur_ack));

            //Pack actual_data_length into the data buffer
            std::memcpy(data+4,&actual_data_length,sizeof(int));

            // Copy remaining data (only actual length)
            std::memcpy(data + 8, readbuffer + 12, actual_data_length);

            // Create the job
            // Verify if data is less than 512 bytes
            if (actual_data_length < 512) {
                
                Job* job=new Job(6, data, total_data_size);

                // Push job to queue safely
                {
                    mtx_jobQueue.lock();
                    jobQueue.push(job);
                    mtx_jobQueue.unlock();

                    cv_jobQueue.notify_one();
                }

            }
            else{

                Job* job=new Job(5, data, total_data_size);

                // Push job to queue safely
                {
                    mtx_jobQueue.lock();
                    jobQueue.push(job);
                    mtx_jobQueue.unlock();

                    cv_jobQueue.notify_one();
                }
            }

            std::cout << "Opcode 5: Received a DATA packet with ackno = " << cur_ack << "\n";

            break;
        }
        case 6: {

            //ERR packet
            size_t max_message_length = BUFFER_SIZE - 4; // Max possible length after opcode
            size_t message_length = strnlen(reinterpret_cast<const char*>(readbuffer + 4), max_message_length);

             if (message_length == 0) {
                std::cout << "Error: Empty filename! Discarding packet." << "\n";

                break;
            }

            // Copy the filename safely
            char message[max_message_length + 1] = {}; // +1 for null terminator
            std::memcpy(message, readbuffer + 4, message_length);
            message[message_length] = '\0'; // Ensure null termination

            // Create Job
            Job* job=new Job(7, message, message_length);

            //push to jobQueue
            {
                mtx_jobQueue.lock();
                jobQueue.push(job);
                mtx_jobQueue.unlock();

                cv_jobQueue.notify_one();
            }


            std::cout << "Opcode 7: Received a ERR packet with message = " << message << "\n";
            break;
        }
        default:
            std::cout << "Invalid Opcode: " << opcode << "\n";
    }

}

void reader_thread(int sockfd) {
    std::cout << "Started Reader Thread..." << "\n";

    socklen_t server_len;
    ssize_t received_bytes;

    while (!stop_thread) {
        // Clear buffer
        bzero(readbuffer, BUFFER_SIZE);

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

        if (select_result < 0) {
            perror("select error");
            continue;
        } else if (select_result == 0) {
            // Timeout occurred, no data to read
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }

        // Locking server address before receiving
        mtx_server_addr.lock();
        server_len = sizeof(server_addr);
        memset(readbuffer,'\0',sizeof(readbuffer));
        received_bytes = recvfrom(sockfd, readbuffer, BUFFER_SIZE, 0, 
                                  (struct sockaddr*)&server_addr, &server_len);
        mtx_server_addr.unlock();

        check_err(received_bytes, "recvfrom failed");

        readbuffer[received_bytes] = '\0';  // Null-terminate received data
        processData();
    }
}