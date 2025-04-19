#pragma once
#include "std_headers.hpp"


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
        data[BUFFER_SIZE]='\0';
    }
};

// Custom comparator: Order by ackno only
struct CompareByAckNo {
    bool operator()(const Packet* p1, const Packet* p2) const {
        return p1->ackno < p2->ackno; // Only compare ackno
    }
};

///////////////////////////////////////////// DS and variables //////////////////////


sockaddr_in server_addr; // stores address of server
std::mutex mtx_server_addr;
std::mutex mtx_jobQueue; //protect jobQueue
std::mutex mtx_window; //protect window
std:: mutex mtx_receiverWindow; //protect receiver Window
std::mutex mtx; //protect should sleep variable
std::condition_variable cv; //signal work->sender thread
std:: condition_variable cv_jobQueue;
std::condition_variable cv_window;
std::condition_variable cv_receiverWindow;

std::atomic<bool> stop_thread(false); //for thread signalling
std::atomic<bool> isTransmissionON(false); //for thread signalling



int windowBits; //number of bits
int windowSize; //windowSize
int SEQN; //total number of sequence numbers
int startSEQN; //starting sequence number of window
int finalSEQN; //ending sequence number of window
int cur_block_ptr; //current block pointer
int cur_ack_no;
int mode; //sender mode=0, receiver mode=1
char* FILENAME; //global character array for storing filename
bool should_sleep; //for ACK notification

std::queue<Job*> jobQueue; //job queue
std::vector<Packet*> window; //window of packets that will be transmitted
std::set<Packet*,CompareByAckNo> receiverWindow; //window of packets that will be maintained while server tries to receive
