
// how to use -->prithi@HP$ ./cli <conFigFilePath>
/* Config File Has the following Info [each in new Line]
Server Ip eg 10.2.10.255
Server Port eg 9999
ServerWindowSize eg 5
Choice eg R or W
FilePath .
*/
#include "c_globals.hpp"

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
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
        return 1;
    }

    Config config;
    if (!readConfigFile(argv[1], config)) {
        return 1;
    }

    std::cout << "Server IP: " << config.serverIp << std::endl;
    std::cout << "Server Port: " << config.serverPort << std::endl;
    std::cout << "Server Window Size: " << config.serverWindowSize << std::endl;
    std::cout << "Choice: " << config.choice << std::endl;
    std::cout << "File Path To Be Read/Written: " << config.filePath << std::endl;
	
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











