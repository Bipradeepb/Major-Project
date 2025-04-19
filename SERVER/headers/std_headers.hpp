#pragma once

#include <stdio.h>
#include <stdlib.h> // malloc , free
#include <string.h>
#include <signal.h>
#include <unistd.h> // system calls
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>   // For shm_open, mmap
#include <semaphore.h>

// cpp
#include <bits/stdc++.h> 
#include <chrono>

//third-party libraries
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/containers/deque.hpp>
#include <boost/interprocess/containers/set.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>

#define BUFFER_SIZE 1024

// Threshold for expiration in milliseconds
const std::chrono::milliseconds threshold(5000);  // Example: 5 seconds