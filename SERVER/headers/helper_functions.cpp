#pragma once
#include "globals.hpp"

using namespace std;

/////////////////////////////// Helper Functions ///////////////////////

std::string getCurrentTime() {
    using namespace std::chrono;

    // Get current time point
    auto now = system_clock::now();

    // Convert to time_t for calendar time
    std::time_t now_c = system_clock::to_time_t(now);
    std::tm* localTime = std::localtime(&now_c);

    // Extract milliseconds
    auto milliseconds_part = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

    // Format the output
    std::ostringstream oss;
    oss << std::put_time(localTime, "%Y-%m-%d %H:%M:%S");  // Format time
    oss << '.' << std::setw(3) << std::setfill('0') << milliseconds_part.count();  // Add milliseconds

    return oss.str();  // e.g., 2025-04-14 16:32:45.123
}

void LOG(std::string msg,string data)
{
    string curTime=getCurrentTime();
    filelog<<"["<<curTime<<"] INFO: "<<msg<<data<<std::endl;
} 


inline void resetJobQueue(Context *ctx)
{
    mtx_jobQueue.lock();
    ctx->jobQueue = ShmJobQueue(ctx->jobQueue.get_allocator());
    mtx_jobQueue.unlock();
}

inline void resetWindow(Context *ctx)
{
    mtx_window.lock();
    ctx->window =  ShmPacketSet(CompareByAckNo(), ctx->window.get_allocator());
    mtx_window.unlock();
}

inline void resetReceiverWindow(Context *ctx)
{
    ctx->receiverWindow = ShmPacketSet(CompareByAckNo(), ctx->receiverWindow.get_allocator());
}

inline void resetDS(Context *ctx)
{
    //reset all Data Structures and variables
    ctx->windowBits=3;
    ctx->windowSize=(1<<2);
    ctx->SEQN=(1<<3);
    ctx->startSEQN=0;
    ctx->finalSEQN=(ctx->startSEQN+ctx->windowSize-1);
    
    ctx->str_block_ptr=0;
    ctx->cur_block_ptr=0;
    ctx->cur_ack_no=0;
    memset(ctx->FILENAME,'\0',256);
    ctx->isTransmissionON=true;
    memset(ctx->clientIp,'\0',INET_ADDRSTRLEN);
    ctx->is_client_ip_set=false;
    ctx->should_sleep=true;

    resetJobQueue(ctx);
    resetWindow(ctx);

    mtx_receiverWindow.lock();
    resetReceiverWindow(ctx);
    mtx_receiverWindow.unlock();

    printf("\nResetting all data structures...\n");
    printf("\nWaiting for new request....\n");
}

extern int sockfd;
//send err packet
void sendERRPacket(Packet packet,Context *config)
{
    struct sockaddr_in clientaddr{};
    clientaddr.sin_family = AF_INET;
    clientaddr.sin_port = htons(config->clientPort);
    inet_pton(AF_INET, config->clientIp, &clientaddr.sin_addr);

    ssize_t sent_bytes = sendto(sockfd, packet.data, BUFFER_SIZE, 0,
                                (const struct sockaddr*)&clientaddr, sizeof(clientaddr));

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
size_t getFileSize(Context *ctx) {
    
    struct stat st;

    if (::stat((const char*)ctx->FILENAME, &st) == 0) {
        return st.st_size;
    }
    perror("Error getting file size");
    exit(EXIT_FAILURE);
}

//format filesize in readable format
std::string formatSize(size_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unitIndex = 0;
    double size = static_cast<double>(bytes);

    while (size >= 1024 && unitIndex < 4) {
        size /= 1024;
        ++unitIndex;
    }

    std::ostringstream out;
    out << std::fixed << std::setprecision(2) << size << " " << units[unitIndex];
    return out.str();
}


size_t readFromFile(char *buffer, size_t maxSize,Context *ctx) {

	//reads 512 bytes atmost from file i.e. only one file block
    
    int fd = open(ctx->FILENAME, O_RDONLY);
    check_err(fd,"Failed to open file.");

    // Seek to the current block pointer
    if (lseek(fd, ctx->cur_block_ptr, SEEK_SET) == -1) {
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
    ctx->cur_block_ptr += bytesRead;
    cout<<"Curr Block Pointer="<<ctx->cur_block_ptr<<"\n";

    return bytesRead;
}

size_t writeToFile(const char *buffer, size_t dataSize, Context* ctx) {

    // string filePath = string(FILENAME);

    string filePath =string(getenv("HOME")) + "/Downloads/" + string(ctx->FILENAME);

    // Open the file for writing, create if it doesn't exist
    int fd = open(filePath.c_str(), O_WRONLY | O_CREAT, 0644);
    check_err(fd,"Failed to open file.");

    // Seek to the current block pointer position
    if (lseek(fd, ctx->cur_block_ptr, SEEK_SET) == -1) {
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
    ctx->cur_block_ptr += bytesWritten;

    return bytesWritten;
}