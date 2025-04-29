
// how to use -->prithi@HP$ ./cli_exe <conFigFilePath> V //V is for verbose and is optional
/* Config File Has the following Info [each in new Line]
Server Ip eg 10.2.10.255
Server Port eg 9999
ServerWindowSize eg 5
Choice eg R or W
FilePath .
*/
#include "c_globals.hpp"
#include "Logger.hpp"

void clientAsReader(const Config& config);
void clientAsWriter(const Config& config);

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
        std::istringstream iss(line);
        switch (lineNumber) {
            case 1: config.serverIp = line; break;
            case 2: config.serverPort = std::stoi(line); break;
            case 3: config.serverWindowSize = std::stoi(line); break;
            case 4:
                if (line.size() == 1 && (line[0] == 'R' || line[0] == 'W')) {
                    config.choice = line[0];
                } else {
                    std::cerr << "Error: Invalid choice value in config file" << std::endl;
                    return false;
                }
                break;
            case 5: config.filePath = line; break;
            default: break;
        }
    }

    if (lineNumber < 5) {
        std::cerr << "Error: Config file missing required values" << std::endl;
        return false;
    }
    return true;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Contents of Config File :-\n";
        std::cout << "The first Line has Switch Ip\n";
        std::cout << "The second Line has Switch Port\n";
        std::cout << "The third Line has R or W to symbolise read or write\n";
        std::cout << "The last line has file path \n";
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
