/*

Header Formats (Similar to TFTP)

          2 bytes    string   1 byte    2 byte
          ------------------------------------------
   RRQ/  | 01/02 |  Filename  |   0  |  WindowSize |
   WRQ    ------------------------------------------
          2 bytes    2 bytes       n bytes
          ---------------------------------
   DATA  | 03    |   Block #  |    Data    |
          ---------------------------------
          2 bytes    2 bytes
          -------------------
   ACK   | 04    |   Block #  |
          --------------------

Note1:    The data field is from zero to 512 bytes long.  If it is 512 bytes
   	  long, the block is not the last block of data; if it is from zero to
   	  511 bytes long, it signals the end of the transfer.

Note2:  a.A WRQ is acknowledged with an ACK packet having a block number of zero.
	b.The WRQ and DATA packets are acknowledged by ACK or ERROR packets
	c.The RRQ and ACK packets are acknowledged by  DATA  or ERROR packets.
	d. All  packets other than duplicate ACK's and those used for termination are acknowledged unless a timeout occurs

*/

#include "c_globals.hpp"
#include "Logger.hpp"

void clientAsReader(const Config& config);
void clientAsWriter(const Config& config);
bool readConfigFile(const std::string& fileName, Config& config);

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Contents of Client Config File :-\n";
        std::cout << "The first Line has Switch Ip\n";
        std::cout << "The second Line has Switch Port\n";
        std::cout << "The third Line has R or W to symbolise read or write\n";
        std::cout << "The fourth line has file path \n";
        std::cout << "The last line(optional) has Timeout in ms\n";
        printf("\nUsage To Enable Verbose Logging to file$	LOG_ON_FILE=1 ./build/cli_exe <config_file> V\n");
		printf("Usage To Enable Verbose Logging to Terminal$	./build/cli_exe <config_file> V\n");
		printf("Usage To Min Logging to Terminal$	./build/cli_exe <config_file>\n");
        return 1;
    }

    if(argc ==3){// checking for verbose logging
		if(std::string(argv[2])== "V"){
			LOG_Verbose = true;
		}
	}
	else if(argc == 2){
		LOG_Verbose = false;
	}

    Config config;
    if (!readConfigFile(argv[1], config)) {
        return 1;
    }

    LOG_TO(LogDestination::TERMINAL_ONLY, "Server IP: " , config.serverIp , "\n")
    LOG_TO(LogDestination::TERMINAL_ONLY,"Server Port: " , config.serverPort , "\n")
    LOG_TO(LogDestination::TERMINAL_ONLY,"Server Window Size: " , config.serverWindowSize , "\n")
    LOG_TO(LogDestination::TERMINAL_ONLY,"Choice: " , config.choice , "\n")
    LOG_TO(LogDestination::TERMINAL_ONLY,"File Path To Be Read/Written: " , config.filePath , "\n")

///////////////////////////////////////////////////////////////////////////////////////////////////

	// User Menu

	if(config.choice == 'R'){
		clientAsReader(config);
	}
	else if(config.choice == 'W') {
		clientAsWriter(config);
	}
	else{
		printf("Wrong choice of %d \n",config.choice);
		return 0;
	}

	return 0;
}

//////////////////////////////////////Utility Functions
bool readConfigFile(const std::string& fileName, Config& config) {
    std::ifstream file(fileName);
    if (!file) {
        std::cerr << "Error: Unable to open config file: " << fileName << std::endl;
        return false;
    }

    std::string line;
    int lineNumber = 0;


    while (std::getline(file, line)) {
        lineNumber++;

        // Trim whitespace (optional enhancement if whitespace padding is allowed)
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        switch (lineNumber) {
            case 1:
                config.serverIp = line;
                break;
            case 2:
                try {
                    config.serverPort = std::stoi(line);
                } catch (...) {
                    std::cerr << "Error: Invalid server port" << std::endl;
                    return false;
                }
                break;
            case 3:
                try {
                    config.serverWindowSize = std::stoi(line);
                } catch (...) {
                    std::cerr << "Error: Invalid window size" << std::endl;
                    return false;
                }
                break;
            case 4:
                if (line.size() == 1 && (line[0] == 'R' || line[0] == 'W')) {
                    config.choice = line[0];
                } else {
                    std::cerr << "Error: Invalid choice value in config file" << std::endl;
                    return false;
                }
                break;
            case 5:
                config.filePath = line;
                break;
            case 6:
                if (!line.empty()) {
                    try {
                        TIMEOUT_MILLI_SEC = std::stoi(line);
                    } catch (...) {
                        std::cerr << "Warning: Invalid timeout value, using default (100)" << std::endl;
                    }
                }
                break;
            default:
                // Ignore extra lines beyond expected config
                break;
        }
    }

    if (lineNumber < 5) {
        std::cerr << "Error: Config file missing required values" << std::endl;
        return false;
    }

    return true;
}
