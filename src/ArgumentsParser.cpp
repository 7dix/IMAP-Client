// ArgumentsParser.cpp
#include "ArgumentsParser.h"
#include <iostream>
#include <unistd.h>
#include <cstdlib>


ProgramOptions ArgumentsParser::parse(int argc, char* argv[]) {
    ProgramOptions options;

    if (argc < 2) {
        printUsage();
        throw std::invalid_argument("No arguments were provided.");
    }

    options.server = argv[1];
    argc--;
    argv++;
    optind = 1;

    int opt;
    while ((opt = getopt(argc, argv, "p:Tc:C:nha:b:o:")) != -1)
    {
        switch (opt) 
        {
            case 'p':
                if (std::atoi(optarg) < 0 || std::atoi(optarg) > 65536) {
                    printUsage();
                    throw std::invalid_argument("Port number is out of range (1-65535).");
                }
                options.port = std::atoi(optarg);
                break;
            case 'T':
                options.useTLS = true;
                break;
            case 'c':
                if (!optarg || !options.useTLS) {
                    printUsage();
                    throw std::invalid_argument("Missing argument -T to use TLS.");
                }
                options.certFile = optarg;
                break;
            case 'C':
                if (!options.useTLS) {
                    printUsage();
                    throw std::invalid_argument("Missing argument -T to use TLS.");
                }
                if (optarg) {
                    options.certDir = optarg;
                }
                break;
            case 'n':
                options.onlyNewMessages = true;
                break;
            case 'h':
                options.headersOnly = true;
                break;
            case 'a':
                options.authFile = optarg;
                break;
            case 'b':
                options.mailbox = optarg;
                break;
            case 'o':
                options.outputDir = optarg;
                break;
            default:
                printUsage();
                throw std::invalid_argument("Unknown argument.");
        }
    }

    if (optind < argc) {
        printUsage();
        throw std::invalid_argument("Unknown argument.");
    }

    // Check for required options after option parsing
    if (options.authFile.empty()) {
        printUsage();
        throw std::invalid_argument("Missing required argument -a.");
    }
    if (options.outputDir.empty()) {
        printUsage();
        throw std::invalid_argument("Missing required argument -o.");
    }


    // Set default port based on TLS usage
    if (options.port == -1) {
        options.port = options.useTLS ? 993 : 143;
    }

    return options;
}

void ArgumentsParser::printUsage() {
    std::cout << std::endl << "Usage: imapcl <server_address> [options]" << std::endl;
    std::cout << "Required arguments:" << std::endl;
    std::cout << "  <server_address>         Address of the IMAP server" << std::endl;
    std::cout << "  -a <auth_file>           Path to the authentication file" << std::endl;
    std::cout << "  -o <output_directory>    Path to the directory for saving emails" << std::endl;
    std::cout << std::endl;
    std::cout << "Optional arguments:" << std::endl;
    std::cout << "  -p <port>                Specify the port number" << std::endl;
    std::cout << "  -T                       Use TLS protocol" << std::endl;
    std::cout << "    -c <cert_file>         Path to the certificate file" << std::endl;
    std::cout << "    -C <cert_directory>    Path to the certificate directory (default is /etc/ssl/certs)" << std::endl;
    std::cout << "  -n                       Download only new messages" << std::endl;
    std::cout << "  -h                       Download only message headers" << std::endl;
    std::cout << "  -b <mailbox>             Name of the mailbox (default is INBOX)" << std::endl;
}