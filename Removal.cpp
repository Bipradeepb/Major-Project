#include <semaphore.h>
#include <fcntl.h>
#include <iostream>
#include <cstdlib>  // For system()

#define SEM_NAME "/my_binary_semaphore"
#define SHM_PATH "/dev/shm/context_shared_mem"

int main() {
    // Remove the named semaphore
    if (sem_unlink(SEM_NAME) == 0) {
        std::cout << "Semaphore " << SEM_NAME << " removed successfully.\n";
    } else {
        perror("sem_unlink failed");
    }

    // Remove the shared memory file
    if (std::system(("rm -f " + std::string(SHM_PATH)).c_str()) == 0) {
        std::cout << "Shared memory " << SHM_PATH << " removed successfully.\n";
    } else {
        std::cerr << "Failed to remove shared memory " << SHM_PATH << "\n";
    }

    return 0;
}
