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


## How to BUILD CLI for all modules(Switch Server Client) :-

--> U should first have the below setup <br>
    prtihijit@hp-250-g6:~$ ls go-back-n <br>
    cleanUp  Client  Readme.md  Server  Switch <br>

    --> cd Client; make clean ; make -j${nproc} ; ./build/cli_exe # then enter to see how to use
    --> cd Server; make clean ; make -j${nproc} ; ./build/ser_exe # then enter to see how to use
    --> cd Switch; make clean ; make -j${nproc} ; ./build/sw_exe # then enter to see how to use

## How to Build GUI ->ClientUI [exe generated inside Client build folder] by:- 

     1. cd Client <br>
     2. prtihijit@hp-250-g6:~/Desktop/go-back-n/Client$ cmake -S ../ClientUI/ -B ./build/ -DCMAKE_PREFIX_PATH=/home/prtihijit/Qt/6.9.0/gcc_64/lib/cmake <br>
     3. prtihijit@hp-250-g6:~/Desktop/go-back-n/Client$ cmake --build ./build <br>

## HOW to Run All the modules(Switch Server Client) with Min Logging to Terminal->

Both switch and client are using config.txt[ also have been provided in this repo as demo] <br>
Change the config files to alter the behaviour and get results as u want

    As per this repo's example setup Follow the below order(top 2 bottom) strictly
        1st Terminal cd inside Server run>  ./build/ser_exe 8000
        2nd Terminal cd inside Server run>  ./build/ser_exe 8001
        3rd Terminal cd inside Switch run>  ./build/sw_exe 9999 ./config.txt
        4th Terminal cd inside Client run>  ./build/cli_exe ./config.txt # for Cli or,
        4th Terminal cd inside Client run>  ./build/ClientUI # for Gui

## How to Run All the modules(Switch Server Client) with Verbose Logging to File ->

    As per this repo's example setup Follow the below order(top 2 bottom) strictly
        1st Terminal cd inside Server run>  LOG_ON_FILE=1 ./build/ser_exe 8000 V
        2nd Terminal cd inside Server run>  LOG_ON_FILE=1 ./build/ser_exe 8001 V
        3rd Terminal cd inside Switch run>  ./build/sw_exe 9999 ./config.txt
        4th Terminal cd inside Client run>  LOG_ON_FILE=1 ./build/cli_exe ./config.txt V # for Cli or,
        4th Terminal cd inside Client run>  LOG_ON_FILE=1 ./build/ClientUI V # for Gui

## How to Run All the modules(Switch Server Client) with Verbose Logging to Terminal ->

    As per this repo's example setup Follow the below order(top 2 bottom) strictly
        1st Terminal cd inside Server run>  ./build/ser_exe 8000 V
        2nd Terminal cd inside Server run>  ./build/ser_exe 8001 V
        3rd Terminal cd inside Switch run>  ./build/sw_exe 9999 ./config.txt
        4th Terminal cd inside Client run>  ./build/cli_exe ./config.txt V # for Cli or,
        4th Terminal cd inside Client run>  ./build/ClientUI V # for Gui


## Distribution of Codebase( - Using tar.gz file )

Zip it --> tar -czvf Go_Back_N_PB.tar.gz /home/prtihijit/Desktop/MajorProect <br>
Unzip it --> tar -xzvf Go_Back_N_PB.tar.gz -C [path where u want to Download] <br>

## Distribution of Software( - Using .deb package)

Go to the ReadMe under pkg_debian folder for More details <br>
