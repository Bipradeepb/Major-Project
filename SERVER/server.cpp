#include "headers/globals.hpp"
#include "headers/helper_functions.cpp"
#include "read.cpp"
#include "send.cpp"
#include "work.cpp"


using namespace std;

int sockfd;

#define SHARED_MEM_NAME "SharedContextMemory"
#define SHARED_MEM_SIZE 262144// 256 KB
// const char* SHARED_MEM_NAME = "/context_shared_mem"; // Shared memory secret key
#define SEM_NAME "/my_binary_semaphore"

// void set_thread_affinity(std::thread& th, int core_id) {
//     cpu_set_t cpuset;
//     CPU_ZERO(&cpuset);
//     CPU_SET(core_id, &cpuset);

//     if (sched_setaffinity(th.native_handle(), sizeof(cpu_set_t), &cpuset) != 0) {
//         std::cerr << "Error setting CPU affinity\n";
//     }
// }

int main(int argc, char **argv)
{
    // checking command line args
	if (argc <2){
		printf("Enter format :- ./server_exe <server_Port>\n");
		exit(1);
	}

	// Try to create the semaphore, if already exists just open it
    sem_t* sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0666, 0);
    if (sem == SEM_FAILED) {
        if (errno == EEXIST) {
            // Semaphore already exists, just open it
            sem = sem_open(SEM_NAME, 0);
        } else {
            perror("sem_open failed");
            return 1;
        }
    }

	
	//creating the socket
	sockfd= socket(AF_INET,SOCK_DGRAM,0);
	check_err(sockfd,"Error in opening UDP socket");
	
	// setting server details
	struct sockaddr_in sa;
	bzero(&sa,sizeof(sa));
	sa.sin_family = AF_INET;//setting address family to IPv4
	sa.sin_port = htons(atoi(argv[1]));
	sa.sin_addr.s_addr=INADDR_ANY;
	
	// binding
	int status= bind(sockfd,(struct sockaddr *) &sa, sizeof(sa));
	check_err(status,"Error in binding");



	/////////////////////// Setting UP Context for ACTIVE / BACKUP /////////////////

    bool is_new = false;
	managed_shared_memory segment(open_or_create, SHARED_MEM_NAME, SHARED_MEM_SIZE);

	// Determine if new or existing
	if (!segment.find<Context>("ctx").first) {
	    is_new = true;
	    VoidAllocator void_alloc(segment.get_segment_manager());
	    segment.construct<Context>("ctx")(void_alloc);
	}

	Context* ctx = segment.find<Context>("ctx").first;

    // If new, initialize memory and Fill up Context For 1st Time
    if (is_new) {

        cout<<"New Server instance is up.......\n";
		
		//Active process 
		//reset DS
		//init threads	
		//optional messages
		printf("\nSetup Finished Sarting 3 threads ...\n\n");

		resetDS(ctx);
				
		std::thread reader(reader_thread,sockfd,ctx);
		std::thread sender(sender_thread,sockfd,ctx);
		std::thread worker(worker_thread,sockfd,ctx);

	    reader.join();
	    worker.join();
	    sender.join();	
		
	}
	else{// This process is a Backup
	
		std::cout<<"Waiting On Semaphore\n";
		sem_wait(sem);  // Block until producer[switch] signals

		cout<<"New Server instance is Up.......\n";
		cout<<"Continuing transmission....\n";
		
		//restart threads 
		//jobs are picked up from where they were left
		//optional messages
		printf("\nContinuing 3 threads in backup...\n\n");
				
		std::thread reader(reader_thread,sockfd,ctx);
		std::thread sender(sender_thread,sockfd,ctx);
		std::thread worker(worker_thread,sockfd,ctx);

	    reader.join();
	    worker.join();
	    sender.join();
	}

	filelog.close();
    close(sockfd);


    return 0;
}