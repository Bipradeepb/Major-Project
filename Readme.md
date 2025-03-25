## About :-

File Transfer System with following components

1. Server (Active)
2. Server (Backup)
3. Switch Emulator cum WatchDog [lies between server layer and Client]
4. Client (UDP + Go-Back-N)

Active and Backup maintains context for client in shared Memory
Backup runs only on getting signal from Switch else wait on semaphore
Active and Backup share same codebase
Protocol header format similar to TFTP
Backup picks up file transfer from where active left using shared context

## Drawbacks -
Support for only single client [multiple client undefined behaviour]
Single point of failure (Switch emulator)
Server and Switch has to be on same machine [as uses shared memory and semaphore]


## How to USE:-
--> U should first have the below setup
prtihijit@hp-250-g6:~$ ls Desktop/MajorProect/
cleanUp  Client  Readme.md  Server  Switch

--> cd Client; make clean ; make -j${nproc} ; ./build/cli_exe # then enter to see how to use
--> cd Server; make clean ; make -j${nproc} ; ./build/ser_exe # then enter to see how to use
--> cd Switch; make clean ; make -j${nproc} ; ./build/sw_exe # then enter to see how to use

Both switch and client are using config.txt[ also have been provided in this repo as demo]
Change the config files to alter the behaviour and get results as u want

    As per this repo's example setup Follow the below order(top 2 bottom) strictly
        1st Terminal run>  ./build/ser_exe 8000
        2nd Terminal run>  ./build/ser_exe 8001
        3rd Terminal run>  ./build/sw_exe 9999 ./config.txt
        4th Terminal run>  ./build/cli_exe ./config.txt

-->Special Note:-
To free the resources( shared Memory / semaphore ) used by the servers and switch
Either run ./cleanUp or
Press CTRL + C on the switch Emultor(if running) [it also frees up the resources]
ie After killing the switch (ur servers are useless kill and restart them)

## Distribution of Codebase( - Using tar.gz file )
Zip it --> tar -czvf Go_Back_N_PB.tar.gz /home/prtihijit/Desktop/MajorProect
Unzip it --> tar -xzvf Go_Back_N_PB.tar.gz -C <path where u want to Download>