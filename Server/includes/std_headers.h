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
#include <fcntl.h>      // For O_* constants
#include <sys/mman.h>   // For shm_open, mmap
#include <sys/stat.h>   // For mode constants

// cpp
#include <bits/stdc++.h> 
#include <chrono>

#define BUFFER_SIZE 1024

// Threshold for expiration in milliseconds
const std::chrono::milliseconds threshold(5000);  // Example: 5 seconds