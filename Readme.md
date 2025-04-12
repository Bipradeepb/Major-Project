## About :-

File Transfer System with following components

1. Server (Active)
2. Server (Backup)
3. Switch Emulator cum WatchDog [lies between server layer and Client]
4. Client (UDP + Go-Back-N)

Active and Backup maintains context for client in shared Memory <br>
Backup runs only on getting signal from Switch else wait on semaphore <br>
Active and Backup share same codebase <br>
Protocol header format similar to TFTP <br>
Backup picks up file transfer from where active left using shared context <br>

## Drawbacks -

1. Support for only single client [multiple client undefined behaviour]
2. Single point of failure (Switch emulator)
3. Server and Switch has to be on same machine [as uses shared memory and semaphore]


## How to USE:-

--> U should first have the below setup <br>
    prtihijit@hp-250-g6:~$ ls Desktop/MajorProect/ <br>
    cleanUp  Client  Readme.md  Server  Switch <br>

    --> cd Client; make clean ; make -j${nproc} ; ./build/cli_exe # then enter to see how to use
    --> cd Server; make clean ; make -j${nproc} ; ./build/ser_exe # then enter to see how to use
    --> cd Switch; make clean ; make -j${nproc} ; ./build/sw_exe # then enter to see how to use

Both switch and client are using config.txt[ also have been provided in this repo as demo] <br>
Change the config files to alter the behaviour and get results as u want

    As per this repo's example setup Follow the below order(top 2 bottom) strictly
        1st Terminal inside Server run>  ./build/ser_exe 8000
        2nd Terminal inside Server run>  ./build/ser_exe 8001
        3rd Terminal inside Switch run>  ./build/sw_exe 9999 ./config.txt
        4th Terminal inside Client run>  ./build/cli_exe ./config.txt

-->Special Note:- <br>
To free the resources( shared Memory / semaphore ) used by the servers and switch <br>
Either run ./cleanUp or <br>
Press CTRL + C on the switch Emultor(if running) [it also frees up the resources] <br>
ie After killing the switch (ur servers are useless kill and restart them) <br>

## Distribution of Codebase( - Using tar.gz file )

Zip it --> tar -czvf Go_Back_N_PB.tar.gz /home/prtihijit/Desktop/MajorProect <br>
Unzip it --> tar -xzvf Go_Back_N_PB.tar.gz -C [path where u want to Download]

## Server Side Logger and Client Side GUI

--> U should first have the below setup[directory structure] <br>

        prtihijit@hp-250-g6:~$ ls Desktop/MajorProect/ <br>
        cleanUp  Client ClientUI Readme.md  Server  Switch <br>

--> build gui ClientUI [exe generated inside Client build folder] by:-  <br>

        1. cd Client <br>
        2. prtihijit@hp-250-g6:~/Desktop/go-back-n/Client$ cmake -S ../ClientUI/ -B ./build/ -DCMAKE_PREFIX_PATH=/home/prtihijit/Qt/6.9.0/gcc_64/lib/cmake <br>
        3. prtihijit@hp-250-g6:~/Desktop/go-back-n/Client$ cmake --build ./build <br>

--> Building [generating exe] for Rest:- Switch Client Server remains same as above

--> Running:-

    As per this repo's example setup Follow the below order(top 2 bottom) strictly
        1st Terminal inside Server run>  LOG_ON=1 ./build/ser_exe 8000 # to generate Log Files under Server/Logs
        Or , 1st Terminal inside Server run>  ./build/ser_exe 8000 # Not generate Log Files 
        2nd Terminal inside Server run>  LOG_ON=1 ./build/ser_exe 8001
        3rd Terminal inside Switch run>  ./build/sw_exe 9999 ./config.txt
        4th Terminal inside Client run>  ./build/ClientUI  #launches the GUI
