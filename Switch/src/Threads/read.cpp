#include "Globals.hpp"

/////////////////////////////////// TFTP Rd Packets
class RRQ_WRQ_Packet{
public:
    //char type; // R for read W for false
    int block_number;
    char* filename;
    char* mode;
};

// Function to extract fields from RRQ/WRQ packet
static RRQ_WRQ_Packet extract_rrq_wrq_packet(const unsigned char* packet) {
    RRQ_WRQ_Packet rrq_wrq;
    //printf("Inside Extract read write extract packet \n");
    int filename_length = strlen((const char*)packet + 2);
    int mode_offset = 2 + filename_length + 1; // Adding 1 to skip null byte after filename
    int mode_length = strlen((const char*)packet + mode_offset);
	
	//printf("File Name length %d\n",filename_length);
	//printf("Mode_length %d\n",mode_length);
	
    rrq_wrq.filename = (char*)malloc(filename_length + 1); // +1 for null terminator
    strncpy(rrq_wrq.filename, (const char*)packet + 2, filename_length);
    rrq_wrq.filename[filename_length] = '\0';
	
	//printf("Extracted file name %s\n",rrq_wrq.filename);
	
    rrq_wrq.mode = (char*)malloc(mode_length + 1); // +1 for null terminator
    strncpy(rrq_wrq.mode, (const char*)packet + mode_offset, mode_length);
    rrq_wrq.mode[mode_length] = '\0';
	
	//printf("Extracted mode name %s\n",rrq_wrq.mode);
    rrq_wrq.block_number=0;
	
    return rrq_wrq;
}

static std::tuple<std::string,const unsigned char * , int> extractPrefix(const unsigned char* data) {
    // Step 1: Convert unsigned char* to char* for easier handling
    const char* ptr = reinterpret_cast<const char*>(data);
    int pad =0;
    // Step 2: Extract ID (up to the first '\0')
    std::string id(ptr);
    pad+=id.size() + 1;
    ptr += id.size() + 1; // Move pointer past the ID and '\0'

    return {id,reinterpret_cast<const unsigned char*>(ptr),pad};
}

static char * addPrefix(const std::string &id, const std::string &fileName, const unsigned char *ptrof, size_t recv_bytes) {
    // Calculate total size needed for new data on the heap
    size_t total_size = id.size() + 1 + fileName.size() + 1 + recv_bytes;
    
    // Allocate memory on the heap
    char *result =(char *)malloc(total_size);

    for(size_t i =0;i<id.size();++i){
        result[i]=id[i];
    }
    result[id.size()]='\0';

    for(size_t i=0;i<fileName.size();++i){
        result[i+id.size()+1]=fileName[i];
    }
    result[id.size()+fileName.size()+1]='\0';

    for(size_t i =0; i<recv_bytes;i++){
        result[i+id.size()+fileName.size()+2] = ptrof[i];
    }

    // //debug:
    // std::cout<<"Printing the Prefix + Tftp\n";
    // for (size_t i =0;i<total_size;++i) {
    //     printf("%02X ", result[i]);
    // }
    // std::cout << "\n";
    
    // Return pointer to the concatenated data
    return result;
}

void reader_thread(int sockfd){

    //setup
    sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    u_char buffer[BUFFER_SIZE];
    char client_ip[INET_ADDRSTRLEN];
    int client_port;
    std::string client_key ,fileName;
    

    while (true) {
        // read from socket()
        bzero(buffer,BUFFER_SIZE);
        ssize_t received_bytes = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &client_len);
        check_err(received_bytes,"recvfrom failed");
        buffer[received_bytes] = '\0';  // Null-terminate the received data
        

        // Convert senders address to a readable IP and port
        bzero(client_ip,INET_ADDRSTRLEN);
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        client_port = ntohs(client_addr.sin_port);

        // Create a unique key associated with every sender
        client_key = std::string(client_ip) + "_" + std::to_string(client_port);
        
        //debug::
        std::cout<<" Just received a packet from "<<client_key<<" and start decoding his packet\n";

        // selecting Active Server
        mtx_ServerList.lock();
        std::string curr_active = server_list[0];
        mtx_ServerList.unlock();

        if(client_key == curr_active){ // recv mssg is from server

            //extract Prefix from recv buffer
            auto [actual_client_key,tftp,pad_size]=extractPrefix(buffer);

            mtx_wd.lock();
            watchDogCnt=0;
            mtx_wd.unlock();

            //create job
            job thejob;
            thejob.destn_type = 'C';
            thejob.destn = actual_client_key;
            thejob.packet_size = received_bytes-pad_size;
            thejob.packet = (char *)malloc(thejob.packet_size);
            memcpy((void *)thejob.packet, tftp, thejob.packet_size);
            
            
            //push job to WorkQ
            {
                std::lock_guard<std::mutex> lock2(mtx_WorkQ);
                WorkQ.push_back(thejob);
            }
            std::cout<<"Mssg recv from Active server | Job created and Push to WorkQ\n";

        }
        else{// recv mssg is from client

            // //debug:
            // std::cout<<"Printing the Exact- Tftp As recv from Client\n";
            // for (int i =0;i<received_bytes;++i) {
            //     printf("%02X ", buffer[i]);
            // }
            // std::cout << "\n";

            mtx_context.lock();
            //create context if new client
            if(client_context.find(client_key)== client_context.end()){// not found
                // if client context not exits ==> 1st packet from  client is always RD/WR
                RRQ_WRQ_Packet rw_packet = extract_rrq_wrq_packet(buffer);
                fileName =std::string(rw_packet.filename);
                client_context[client_key] = fileName;
                //clean up
                free(rw_packet.filename);
                free(rw_packet.mode);
            }
            else{// context exists
                fileName = client_context[client_key];
            }
            mtx_context.unlock();

            // create job;
            job thejob;
            thejob.destn_type = 'S';
            thejob.destn = curr_active;            
            thejob.packet = addPrefix(client_key,fileName,buffer,received_bytes);
            thejob.packet_size = received_bytes+client_key.size()+fileName.size()+2;

            //push job to WorkQ
            {
                std::lock_guard<std::mutex> lock2(mtx_WorkQ);
                WorkQ.push_back(thejob);
            }
            std::cout<<"Mssg recv from Client | Job created and Push to WorkQ\n";
        }

        cv_work.notify_one(); // notifies the frwd thread that WorQ has data
    }
}