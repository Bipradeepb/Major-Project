## Installing the pkg :-

        sudo apt install ./udp-gbn-suite.deb

## Removing the pkg :-

        sudo apt purge udp-gbn-suite

## Norms of Usage :-

1. Ensure /opt/gobackn exists with Server Switch Client folders [exe present under build folder]
2. Note same Log file is reused
3. Keep the files used for Transfering Under Server/Demo and Client/Demo
4. Use config.txt under Client and Switch and Server To set configurations of the whole suite
5. Do not touch the lib folder [contains linker and so files]
6. While reading a file ensure a file of same name is not present [else corrupt]
7. Run /opt/gobackn/runCleanUp [after killing the Servers] 


## HOW to Run All the modules(Switch Server Client) with Min Logging to Terminal->

---> cd /opt/gobackn <br>

   As per this repo's example setup Follow the below order(top 2 bottom) strictly
        1st Terminal cd inside Switch run>  ./build/runSwitch  ./config.txt
        2nd Terminal cd inside Server run>  ./build/runServer ./config_a.txt
        3rd Terminal cd inside Server run>  ./build/runServer ./config_b.txt
        4th Terminal cd inside Client run>  ./build/runCLI ./config.txt # for Cli or,
        4th Terminal cd inside Client run>  ./build/runGUI # for Gui

## How to Run All the modules(Switch Server Client) with Verbose Logging to File ->

---> cd /opt/gobackn <br>

   As per this repo's example setup Follow the below order(top 2 bottom) strictly
        1st Terminal cd inside Switch run>  LOG_ON_FILE=1 ./build/runSwitch  ./config.txt V
        2nd Terminal cd inside Server run>  LOG_ON_FILE=1 ./build/runServer ./config_a.txt V
        3rd Terminal cd inside Server run>  LOG_ON_FILE=1 ./build/runServer ./config_b.txt V
        4th Terminal cd inside Client run>  LOG_ON_FILE=1 ./build/runCLI ./config.txt V # for Cli or,
        4th Terminal cd inside Client run>  LOG_ON_FILE=1 ./build/runGUI V # for Gui

## How to Run All the modules(Switch Server Client) with Verbose Logging to Terminal ->

---> cd /opt/gobackn <br>

   As per this repo's example setup Follow the below order(top 2 bottom) strictly
        1st Terminal cd inside Switch run>  ./build/runSwitch  ./config.txt V
        2nd Terminal cd inside Server run>  ./build/runServer ./config_a.txt V
        3rd Terminal cd inside Server run>  ./build/runServer ./config_b.txt V
        4th Terminal cd inside Client run>  ./build/runCLI ./config.txt V # for Cli or,
        4th Terminal cd inside Client run>  ./build/runGUI V # for Gui
