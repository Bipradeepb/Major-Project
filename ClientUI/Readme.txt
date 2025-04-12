--> U should first have the below setup
prtihijit@hp-250-g6:~$ ls Desktop/MajorProect/
cleanUp Client ClientUI Readme.md Server Switch


Assume U are running this UI exe from ClientUI folder level
ie prtihijit@hp-250-g6:~/Desktop/go-back-n/ClientUI$ ./build/Desktop_Qt_6_9_0-Debug/ClientUI

Then every file path must be wrt to ClientUI level[ClientUI and Client are at same level]
Config File Path[hardcoded in UI code] = ../Client/config.txt
Client Exe Path =  ../Client/build/cli_exe
File Path Written in Config File  = ../CLient/Demo/<fileName>
