#include <bits/stdc++.h>

std::pair<unsigned char*,size_t> readFileBlock(const std::string& fileName, int block_num, const std::string& mode) {
    // Determine the file opening mode based on the input "mode"
    const char* fileMode = (mode == "octet") ? "rb" : "r";  // "rb" for binary, "r" for text

    // Open the file in the specified mode
    FILE* file = fopen(fileName.c_str(), fileMode);
    if (!file) {
        std::cerr << "Error opening file: " << fileName << std::endl;
        return {nullptr,0};
    }

    //std::cout<<"File size is "<<ftell(file) << " fileMode "<<fileMode<<" fileName = "<< fileName.c_str()<<"\n";
    
    // Calculate the offset and seek to that position in the file
    long offset = block_num * 512;
    if (fseek(file, offset, SEEK_SET) != 0) {
        std::cerr << "Error seeking in file." << std::endl;
        fclose(file);
        return {nullptr,0};
    }

    // Check if the file pointer is beyond the end of the file
    long currentPos = ftell(file);
    fseek(file, 0, SEEK_END);  // Go to the end of the file to find its length
    long fileSize = ftell(file);

    std::cerr<<"currentPos = "<<currentPos<<" file Size = "<<fileSize<<"\n";
    if (currentPos >= fileSize) {
        
        std::cerr << "Attempted to read beyond the end of file." << std::endl;
        std::cerr << "fileName = "<<fileName <<" block_num = "<<block_num<<" mode = "<<mode<<std::endl;
        fclose(file);
        return {nullptr,0};
    }

    // Go back to the original position after checking file size
    fseek(file, currentPos, SEEK_SET);
    
    // Allocate memory for reading 512 bytes
    unsigned char* buffer = new unsigned char[512];
    std::memset(buffer, 0, 512);  // Initialize buffer with 0

    // Read 512 bytes into buffer
    size_t bytesRead = fread(buffer, sizeof(unsigned char), 512, file);
    if (bytesRead != 512) {
        std::cerr << "Warning: Could not read full 512 bytes, read " << bytesRead << " bytes." << std::endl;
    }

    // Close the file
    fclose(file);

    return {buffer,bytesRead};
}

int main(){

    auto temp = readFileBlock("c.txt",0,"octet");
    std::cout<<"Read = \n"<<temp.first<<"\n";

    return 0;
}#include <bits/stdc++.h>

std::pair<unsigned char*,size_t> readFileBlock(const std::string& fileName, int block_num, const std::string& mode) {
    // Determine the file opening mode based on the input "mode"
    const char* fileMode = (mode == "octet") ? "rb" : "r";  // "rb" for binary, "r" for text

    // Open the file in the specified mode
    FILE* file = fopen(fileName.c_str(), fileMode);
    if (!file) {
        std::cerr << "Error opening file: " << fileName << std::endl;
        return {nullptr,0};
    }

    //std::cout<<"File size is "<<ftell(file) << " fileMode "<<fileMode<<" fileName = "<< fileName.c_str()<<"\n";
    
    // Calculate the offset and seek to that position in the file
    long offset = block_num * 512;
    if (fseek(file, offset, SEEK_SET) != 0) {
        std::cerr << "Error seeking in file." << std::endl;
        fclose(file);
        return {nullptr,0};
    }

    // Check if the file pointer is beyond the end of the file
    long currentPos = ftell(file);
    fseek(file, 0, SEEK_END);  // Go to the end of the file to find its length
    long fileSize = ftell(file);

    std::cerr<<"currentPos = "<<currentPos<<" file Size = "<<fileSize<<"\n";
    if (currentPos >= fileSize) {
        
        std::cerr << "Attempted to read beyond the end of file." << std::endl;
        std::cerr << "fileName = "<<fileName <<" block_num = "<<block_num<<" mode = "<<mode<<std::endl;
        fclose(file);
        return {nullptr,0};
    }

    // Go back to the original position after checking file size
    fseek(file, currentPos, SEEK_SET);
    
    // Allocate memory for reading 512 bytes
    unsigned char* buffer = new unsigned char[512];
    std::memset(buffer, 0, 512);  // Initialize buffer with 0

    // Read 512 bytes into buffer
    size_t bytesRead = fread(buffer, sizeof(unsigned char), 512, file);
    if (bytesRead != 512) {
        std::cerr << "Warning: Could not read full 512 bytes, read " << bytesRead << " bytes." << std::endl;
    }

    // Close the file
    fclose(file);

    return {buffer,bytesRead};
}

int main(){

    auto temp = readFileBlock("c.txt",0,"octet");
    std::cout<<"Read = \n"<<temp.first<<"\n";

    return 0;
}#include <bits/stdc++.h>

std::pair<unsigned char*,size_t> readFileBlock(const std::string& fileName, int block_num, const std::string& mode) {
    // Determine the file opening mode based on the input "mode"
    const char* fileMode = (mode == "octet") ? "rb" : "r";  // "rb" for binary, "r" for text

    // Open the file in the specified mode
    FILE* file = fopen(fileName.c_str(), fileMode);
    if (!file) {
        std::cerr << "Error opening file: " << fileName << std::endl;
        return {nullptr,0};
    }

    //std::cout<<"File size is "<<ftell(file) << " fileMode "<<fileMode<<" fileName = "<< fileName.c_str()<<"\n";
    
    // Calculate the offset and seek to that position in the file
    long offset = block_num * 512;
    if (fseek(file, offset, SEEK_SET) != 0) {
        std::cerr << "Error seeking in file." << std::endl;
        fclose(file);
        return {nullptr,0};
    }

    // Check if the file pointer is beyond the end of the file
    long currentPos = ftell(file);
    fseek(file, 0, SEEK_END);  // Go to the end of the file to find its length
    long fileSize = ftell(file);

    std::cerr<<"currentPos = "<<currentPos<<" file Size = "<<fileSize<<"\n";
    if (currentPos >= fileSize) {
        
        std::cerr << "Attempted to read beyond the end of file." << std::endl;
        std::cerr << "fileName = "<<fileName <<" block_num = "<<block_num<<" mode = "<<mode<<std::endl;
        fclose(file);
        return {nullptr,0};
    }

    // Go back to the original position after checking file size
    fseek(file, currentPos, SEEK_SET);
    
    // Allocate memory for reading 512 bytes
    unsigned char* buffer = new unsigned char[512];
    std::memset(buffer, 0, 512);  // Initialize buffer with 0

    // Read 512 bytes into buffer
    size_t bytesRead = fread(buffer, sizeof(unsigned char), 512, file);
    if (bytesRead != 512) {
        std::cerr << "Warning: Could not read full 512 bytes, read " << bytesRead << " bytes." << std::endl;
    }

    // Close the file
    fclose(file);

    return {buffer,bytesRead};
}

int main(){

    auto temp = readFileBlock("c.txt",0,"octet");
    std::cout<<"Read = \n"<<temp.first<<"\n";

    return 0;
}#include <bits/stdc++.h>

std::pair<unsigned char*,size_t> readFileBlock(const std::string& fileName, int block_num, const std::string& mode) {
    // Determine the file opening mode based on the input "mode"
    const char* fileMode = (mode == "octet") ? "rb" : "r";  // "rb" for binary, "r" for text

    // Open the file in the specified mode
    FILE* file = fopen(fileName.c_str(), fileMode);
    if (!file) {
        std::cerr << "Error opening file: " << fileName << std::endl;
        return {nullptr,0};
    }

    //std::cout<<"File size is "<<ftell(file) << " fileMode "<<fileMode<<" fileName = "<< fileName.c_str()<<"\n";
    
    // Calculate the offset and seek to that position in the file
    long offset = block_num * 512;
    if (fseek(file, offset, SEEK_SET) != 0) {
        std::cerr << "Error seeking in file." << std::endl;
        fclose(file);
        return {nullptr,0};
    }

    // Check if the file pointer is beyond the end of the file
    long currentPos = ftell(file);
    fseek(file, 0, SEEK_END);  // Go to the end of the file to find its length
    long fileSize = ftell(file);

    std::cerr<<"currentPos = "<<currentPos<<" file Size = "<<fileSize<<"\n";
    if (currentPos >= fileSize) {
        
        std::cerr << "Attempted to read beyond the end of file." << std::endl;
        std::cerr << "fileName = "<<fileName <<" block_num = "<<block_num<<" mode = "<<mode<<std::endl;
        fclose(file);
        return {nullptr,0};
    }

    // Go back to the original position after checking file size
    fseek(file, currentPos, SEEK_SET);
    
    // Allocate memory for reading 512 bytes
    unsigned char* buffer = new unsigned char[512];
    std::memset(buffer, 0, 512);  // Initialize buffer with 0

    // Read 512 bytes into buffer
    size_t bytesRead = fread(buffer, sizeof(unsigned char), 512, file);
    if (bytesRead != 512) {
        std::cerr << "Warning: Could not read full 512 bytes, read " << bytesRead << " bytes." << std::endl;
    }

    // Close the file
    fclose(file);

    return {buffer,bytesRead};
}

int main(){

    auto temp = readFileBlock("c.txt",0,"octet");
    std::cout<<"Read = \n"<<temp.first<<"\n";

    return 0;
}#include <bits/stdc++.h>

std::pair<unsigned char*,size_t> readFileBlock(const std::string& fileName, int block_num, const std::string& mode) {
    // Determine the file opening mode based on the input "mode"
    const char* fileMode = (mode == "octet") ? "rb" : "r";  // "rb" for binary, "r" for text

    // Open the file in the specified mode
    FILE* file = fopen(fileName.c_str(), fileMode);
    if (!file) {
        std::cerr << "Error opening file: " << fileName << std::endl;
        return {nullptr,0};
    }

    //std::cout<<"File size is "<<ftell(file) << " fileMode "<<fileMode<<" fileName = "<< fileName.c_str()<<"\n";
    
    // Calculate the offset and seek to that position in the file
    long offset = block_num * 512;
    if (fseek(file, offset, SEEK_SET) != 0) {
        std::cerr << "Error seeking in file." << std::endl;
        fclose(file);
        return {nullptr,0};
    }

    // Check if the file pointer is beyond the end of the file
    long currentPos = ftell(file);
    fseek(file, 0, SEEK_END);  // Go to the end of the file to find its length
    long fileSize = ftell(file);

    std::cerr<<"currentPos = "<<currentPos<<" file Size = "<<fileSize<<"\n";
    if (currentPos >= fileSize) {
        
        std::cerr << "Attempted to read beyond the end of file." << std::endl;
        std::cerr << "fileName = "<<fileName <<" block_num = "<<block_num<<" mode = "<<mode<<std::endl;
        fclose(file);
        return {nullptr,0};
    }

    // Go back to the original position after checking file size
    fseek(file, currentPos, SEEK_SET);
    
    // Allocate memory for reading 512 bytes
    unsigned char* buffer = new unsigned char[512];
    std::memset(buffer, 0, 512);  // Initialize buffer with 0

    // Read 512 bytes into buffer
    size_t bytesRead = fread(buffer, sizeof(unsigned char), 512, file);
    if (bytesRead != 512) {
        std::cerr << "Warning: Could not read full 512 bytes, read " << bytesRead << " bytes." << std::endl;
    }

    // Close the file
    fclose(file);

    return {buffer,bytesRead};
}

int main(){

    auto temp = readFileBlock("c.txt",0,"octet");
    std::cout<<"Read = \n"<<temp.first<<"\n";

    return 0;
}