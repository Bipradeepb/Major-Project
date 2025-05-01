#include "globals.hpp"
#include "packets.hpp"
#include "Logger.hpp"

std::pair<unsigned char*,size_t> readFileBlock(const std::string& fileName, int block_num, const std::string& mode) {
    // Determine the file opening mode based on the input "mode"
    const char* fileMode = (mode == "octet") ? "rb" : "r";  // "rb" for binary, "r" for text

    // Open the file in the specified mode
    FILE* file = fopen(fileName.c_str(), fileMode);
    if (!file) {
        std::cerr << "Error opening file: " << fileName << std::endl;
        return {nullptr,0};
    }

    //LOG("File size is "<<ftell(file) << " fileMode "<<fileMode<<" fileName = "<< fileName.c_str()<<"\n";

    // Calculate the offset and seek to that position in the file
    long offset = block_num * 512;
    if (fseek(file, offset, SEEK_SET) != 0) {
        std::cerr << "Error seeking in file." << std::endl;
        fclose(file);
        return {nullptr,0};
    }

    // Check if the file pointer is beyond the end of the file
    long currentPos = ftell(file);
    fseek(file, 0, SEEK_END);  // Go to the end of the file to find its length
    long fileSize = ftell(file);

    if (currentPos >= fileSize) {
        //std::cerr<<"currentPos = "<<currentPos<<" file Size = "<<fileSize<<"\n";
        LOG("Note: Attempted to read beyond the end of file => blk = ",block_num-1," Earlier Sent is the last data packet\n");
        //std::cerr << "fileName = "<<fileName <<" block_num = "<<block_num<<" mode = "<<mode<<std::endl;
        fclose(file);
        return {nullptr,0};
    }

    // Go back to the original position after checking file size
    fseek(file, currentPos, SEEK_SET);

    // Allocate memory for reading 512 bytes
    unsigned char* buffer = new unsigned char[512];
    std::memset(buffer, 0, 512);  // Initialize buffer with 0

    // Read 512 bytes into buffer
    size_t bytesRead = fread(buffer, sizeof(unsigned char), 512, file);
    if (bytesRead != 512) {
        LOG("Note: Read from file " ,bytesRead ," Bytes (<512 B) => curr blk = ",block_num," is the last data packet\n");
    }

    // Close the file
    fclose(file);

    return {buffer,bytesRead};
}


void forwardThread(int sockfd, Context* ctx) {
    struct sockaddr_in clientAddr;
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(ctx->clientPort);
    inet_pton(AF_INET, ctx->clientIp, &clientAddr.sin_addr);
    int limit;

    while(true){

      mtx.lock();
      limit = ctx->current_blk + ctx->WindowSize -1;
      mtx.unlock();

    Windowloop:
        mtx.lock();
        auto blk = readFileBlock(ctx->fileName, ctx->current_blk , "octet");
        mtx.unlock();

        if(blk.second ==0){// Out of Bound File Access[current blk =last data blk +1]
            loop:
            mtx.lock();
            if(flag_Ack_Recv == true){//set from read thread inside ACK recv blk
                if(read_thread_end == true){
                    mtx.unlock();
                    break;
                }
                else{
                    flag_Ack_Recv = false;
                    mtx.unlock();
                    continue;
                }
            }
            mtx.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(TIMEOUT_MILLI_SEC+2));
            goto loop;// waiting for ack
        }

        mtx.lock();
        LOG("Building Data Packet for blk = ",ctx->current_blk,"\n");
        u_char * packet = build_data_packet(ctx->current_blk,blk.first,blk.second);
        sendto(sockfd, packet, blk.second + 4, 0, (struct sockaddr*)&clientAddr, sizeof(clientAddr));
        if(blk.second <512){
            last_D_blk = ctx->current_blk;
        }
        ctx->current_blk ++;

        //free resource
        free(packet);
        delete [] (blk.first);

        if(ctx->current_blk <= limit){
            if(flag_Ack_Recv){
                LOG("Midway of send Window Updated current_blk = ",ctx->current_blk,"due to ACK\n");
                flag_Ack_Recv = false;
                limit = ctx->current_blk + ctx->WindowSize -1; // New Limit [current_blk is updated in ReadThread]
            }
            mtx.unlock();

            goto Windowloop;
        }
        mtx.unlock(); // Finished Transmitting Entire Window

        //Before Going to Next Window Wait For ACK recv / Ack Timeout
        ack_loop:
        mtx.lock();
        if(flag_Ack_Recv != true){
            mtx.unlock();
            goto ack_loop;
        }
        flag_Ack_Recv = false;
        mtx.unlock();

    }// Loop Back to transmit Next Window

    // Reach Here ==> Full File Transfer Complete

}
