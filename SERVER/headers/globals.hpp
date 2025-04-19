#pragma once
#include "std_headers.hpp"

using namespace boost::interprocess;

////////////////////////////////Classes////////////////////////////////////

class Job {
public:
    int jobId;
    char* data;      // Deep copy of data
    size_t dataSize; // Store the size of the data

    // Constructor (Deep Copy)
    Job(int id, const char* inputData, size_t size) : jobId(id), dataSize(size) {
        data = new char[dataSize + 1];  // +1 for null terminator (if needed)
        std::memset(data,'\0',dataSize);
        std::memcpy(data, inputData, size);
        data[size] = '\0';  // Ensure null termination for safety
    }

    // Copy Constructor (Deep Copy)
    Job(const Job& other) : jobId(other.jobId), dataSize(other.dataSize) {
        data = new char[dataSize + 1];
        std::memset(data,'\0',dataSize);
        std::memcpy(data, other.data, dataSize);
        data[dataSize] = '\0';
    }

    // Destructor (Frees Memory)
    ~Job() {
        delete[] data;
    }

    // Get Data as a Null-Terminated String (For Debugging)
    std::string getString() const {
        return std::string(data, dataSize);
    }
};

class Packet {
public:
    int ackno;
    char data[BUFFER_SIZE];
    int dataSize;

    Packet(int a, const char* d,int size) : ackno(a),dataSize(size) {
        std::memset(data,'\0',BUFFER_SIZE);
        std::memcpy(data, d, BUFFER_SIZE);
        data[BUFFER_SIZE - 1] = '\0'; // Ensure null termination
    }
};


// Custom comparator: Order by ackno only
struct CompareByAckNo {
    bool operator()(const Packet& p1, const Packet& p2) const {
        return p1.ackno < p2.ackno; // Only compare ackno
    }
};

///////////////////////////// Define allocators /////////////////////////////
typedef managed_shared_memory::segment_manager SegmentManager;
typedef allocator<void, SegmentManager> VoidAllocator;

typedef allocator<Job, SegmentManager> JobAllocator;
// typedef allocator<Packet, SegmentManager> PacketAllocator;
typedef allocator<Packet, SegmentManager> PacketSetAllocator;


typedef deque<Job, JobAllocator> ShmJobQueue;
// typedef vector<Packet, PacketAllocator> ShmPacketVector;
typedef set<Packet, CompareByAckNo, PacketSetAllocator> ShmPacketSet;



///////////////////////////////////////////// DS and variables //////////////////////


std::mutex mtx_jobQueue; //protect jobQueue
std::mutex mtx_window; //protect window
std::mutex mtx_receiverWindow; //protect receiver Window
std::mutex mtx; //protect should sleep variable
std::condition_variable cv; //signal work->sender thread
std::condition_variable cv_jobQueue;
std::condition_variable cv_window;
std::condition_variable cv_receiverWindow;
std::ofstream filelog("../LOGS/logs.txt"); //open file for logging

struct Context{

    int windowBits; //number of bits
    int windowSize; //windowSize
    int SEQN; //total number of sequence numbers
    int startSEQN; //starting sequence number of window
    int finalSEQN; //ending sequence number of window
    int cur_ack_no;
    int mode; //sender mode=0, receiver mode=1

    int str_block_ptr; //block pointer before a job begins
    int cur_block_ptr; //current block pointer
    char FILENAME[256]; //global character array for storing filename
    char clientIp[INET_ADDRSTRLEN];
    int clientPort;

    bool is_client_ip_set;   // Flag to check if initialized
    std::atomic<bool> isTransmissionON{false}; //for thread signalling
    bool should_sleep; //for ACK notification; //for ACK notification

    Context(const VoidAllocator& void_alloc)
        : jobQueue(void_alloc),
          window(void_alloc),
          receiverWindow(CompareByAckNo(), void_alloc) {}



    ShmJobQueue jobQueue;  //job queue
    ShmPacketSet window; //window of packets that will be transmitted
    ShmPacketSet receiverWindow; //window of packets that will be maintained while server tries to receive

};

