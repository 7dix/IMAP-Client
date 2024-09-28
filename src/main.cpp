// main.cpp
#include <iostream>
#include <stdlib.h>

#include "ArgumentsParser.h"
#include "AuthReader.h"

int main(int argc, char* argv[]) {
    ArgumentsParser argumentsParser;
    ProgramOptions options = argumentsParser.parse(argc, argv);

    AuthReader authReader(options.authFile);
    AuthData authData = authReader.read();
    std::cout << "Name: '" << authData.username << "' password: '" << authData.password << "'" << std::endl;

    if (options.useTLS) {
        std::cout << "Loading SSL certificates.." << std::endl;
    }
    // Read authentication file


    std::cout << "Hello, World!" << std::endl;
}