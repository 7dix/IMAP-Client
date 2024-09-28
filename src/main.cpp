// main.cpp
#include <iostream>
#include <stdlib.h>

#include "ArgumentsParser.h"
#include "AuthReader.h"

int main(int argc, char* argv[]) {
    ArgumentsParser argumentsParser;
    ProgramOptions options = argumentsParser.parse(argc, argv);

    exit(EXIT_SUCCESS);
    AuthReader authReader(options.authFile);
    AuthData authData = authReader.read();
    std::cout << "Loaded: " << authData.username << " " << authData.password << std::endl;

    if (options.useTLS) {
        std::cout << "Loading SSL certificates.." << std::endl;
    }
    // Read authentication file


    std::cout << "Hello, World!" << std::endl;
}