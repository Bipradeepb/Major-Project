## Installing the pkg :-

        sudo apt install ./udp-ft-suite.deb

## Removing the pkg :-

        sudo apt purge udp-ft-suite

## Norms of Usage :-

1. Ensure /opt/gobackn exists with Server Switch Client folders [exe present under build folder]
2. Logs of Server will appear under Server/Logs [Note same file is reused]
3. Keep the files used for Transfering Under Server/Demo and Client/Demo
4. Use config.txt under Client and Switch To set configurations of the whole suite
5. Do not touch the lib folder [used by GUI]
6. While reading a file ensure a file of same name is not present [else corrupt] 

## Without Using Logger (on Server Side) and GUI (on Client side)

1. cd /opt/gobackn

2. As per example setup Follow the below order(top 2 bottom) strictly <br>

        1st Terminal cd inside Server run>  ./build/runServer 8000
        2nd Terminal cd inside Server run>  ./build/runServer 8001
        3rd Terminal cd inside Switch run>  ./build/runSwitch 9999 ./config.txt
        4th Terminal cd inside Client run>  ./build/runCLI ./config.txt

## With Using Logger (on Server Side) and GUI (on Client side)

1. cd /opt/gobackn

2. As per this repo's example setup Follow the below order(top 2 bottom) strictly <br>

        1st Terminal cd inside Server run>  LOG_ON=1 ./build/runServer 8000 # to generate Log Files under Server/Logs
        Or , 1st Terminal cd inside Server run>  ./build/runServer 8000 # Not generate Log Files 
        2nd Terminal cd inside Server run>  LOG_ON=1 ./build/runServer 8001
        3rd Terminal cd inside Switch run>  ./build/runSwitch 9999 ./config.txt
        4th Terminal cd inside Client run>  ./build/runGUI  #launches the GUI
