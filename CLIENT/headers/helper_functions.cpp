#pragma once
#include "globals.hpp"

using namespace std;

/////////////////////////////// Helper Functions ///////////////////////

inline void resetJobQueue()
{
    mtx_jobQueue.lock();
    while(!jobQueue.empty())
    {
        Job* job=jobQueue.front();
        jobQueue.pop();
        delete job;
    }
    mtx_jobQueue.unlock();
}

inline void resetWindow()
{
    mtx_window.lock();
    while(!window.empty())
    {
        Packet* pkt=window.back();
        window.pop_back();
        delete pkt;
    }
    mtx_window.unlock();
}

inline void resetReceiverWindow()
{
    for(Packet* pkt:receiverWindow)
    {
        delete pkt;
    }

    receiverWindow.clear();
}

inline void resetDS()
{
    //reset all Data Structures and variables
    windowBits=3;
    windowSize=(1<<2);
    SEQN=(1<<3);
    startSEQN=0;
    finalSEQN=(startSEQN+windowSize-1);
    cur_block_ptr=0;
    cur_ack_no=0;
    FILENAME=(char*)malloc(200);
    memset(FILENAME,'\0',sizeof(FILENAME));
    isTransmissionON=true;
    should_sleep=true;

    resetJobQueue();
    resetWindow();

    mtx_receiverWindow.lock();
    resetReceiverWindow();
    mtx_receiverWindow.unlock();

    printf("\nResetting all data structures...\n");
}

void transmitACK(Packet* packet,int sockfd)
{
    mtx_server_addr.lock();
    ssize_t sent_bytes = sendto(sockfd, packet->data, BUFFER_SIZE, 0,
                                (const struct sockaddr*)&server_addr, sizeof(server_addr));
    mtx_server_addr.unlock();

    if (sent_bytes == -1)
    {
        perror("sendto failed");
        exit(1);
    }
}

extern int sockfd;
//send err packet
void sendERRPacket(Packet* packet)
{
    mtx_server_addr.lock();
    ssize_t sent_bytes = sendto(sockfd, packet->data, BUFFER_SIZE, 0,
                                (const struct sockaddr*)&server_addr, sizeof(server_addr));
    mtx_server_addr.unlock();

    if (sent_bytes == -1)
    {
        perror("sendto failed");
        exit(1);
    }
}

inline void check_err(int fd,std::string mssg){
	if(fd <0){
		perror(mssg.c_str());
		close(fd);
		exit(1);
	}	
}

// Function to get the file size
size_t getFileSize() {
    
    struct stat st;

    if (::stat((const char*)FILENAME, &st) == 0) {
        return st.st_size;
    }
    perror("Error getting file size");
    exit(EXIT_FAILURE);
}

size_t readFromFile(char *buffer, size_t maxSize) {

	//reads 512 bytes atmost from file i.e. only one file block
    
    int fd = open(FILENAME, O_RDONLY);
    check_err(fd,"Failed to open file.");

    // Seek to the current block pointer
    if (lseek(fd, cur_block_ptr, SEEK_SET) == -1) {
        perror("Error seeking file");
        close(fd);
        return 0;
    }

    // Read up to maxSize bytes
    size_t bytesRead = read(fd, buffer, maxSize);
    if (bytesRead < 0) {
        perror("Error reading file");
        close(fd);

        exit(1);
    }

    close(fd);

    // Update the block pointer for the next read
    cur_block_ptr += bytesRead;

    return bytesRead;
}

size_t writeToFile(const char *buffer, size_t dataSize) {

	string filePath = string(FILENAME);

    // Open the file for writing, create if it doesn't exist
    int fd = open(filePath.c_str(), O_WRONLY | O_CREAT, 0644);
    check_err(fd,"Failed to open file.");

    // Seek to the current block pointer position
    if (lseek(fd, cur_block_ptr, SEEK_SET) == -1) {
        perror("Error seeking file");
        close(fd);
        return 0;
    }

    // Write up to dataSize bytes
    ssize_t bytesWritten = write(fd, buffer, dataSize);
    if (bytesWritten < 0) {
        perror("Error writing file");
        close(fd);
        return 0;
    }

    // Close the file
    close(fd);

    // Update the block pointer for the next write
    cur_block_ptr += bytesWritten;

    return bytesWritten;
}