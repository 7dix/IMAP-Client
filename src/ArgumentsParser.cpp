// ArgumentsParser.cpp
#include "ArgumentsParser.h"
#include <iostream>
#include <getopt.h>
#include <cstdlib>


ProgramOptions ArgumentsParser::parse(int argc, char* argv[]) {
    ProgramOptions options;

    options.port = -1; // Not default port, used for checking changes
    options.useTLS = false; // Default no TLS
    options.certDir = "/etc/ssl/certs"; // Default cert dir
    options.onlyNewMessages = false; // Default all messages
    options.headersOnly = false; // Default full messages
    options.mailbox = "INBOX"; // Default mailbox

    int opt = 1;
    while ((opt = getopt(argc, argv, "p:Tc:C:nha:b:o:")) != -1) 
    {
        switch (opt) {
            case 'p':
                std::cout << "Option -p with value " << optarg << std::endl; // Debug
                if (std::atoi(optarg) < 0 || std::atoi(optarg) > 65536) {
                    std::cerr << "Číslo portu je mimo rozsah (0-65535)" << std::endl;
                    printUsage();
                    std::exit(EXIT_FAILURE);
                }
                options.port = std::atoi(optarg);
                break;
            case 'T':
                std::cout << "Option -T" << std::endl; // Debug
                options.useTLS = true;
                break;
            case 'c':
                std::cout << "Option -c with value " << optarg << std::endl; // Debug
                if (!optarg || !options.useTLS) {
                    printUsage();
                    std::exit(EXIT_FAILURE);
                }
                options.certFile = optarg;
                break;
            case 'C':
                std::cout << "Option -C with value " << optarg << std::endl; // Debug
                if (!options.useTLS) {
                    printUsage();
                    std::exit(EXIT_FAILURE);
                }
                if (optarg) {
                    options.certDir = optarg;
                }
                break;
            case 'n':
                std::cout << "Option -n" << std::endl; // Debug
                options.onlyNewMessages = true;
                break;
            case 'h':
                std::cout << "Option -h" << std::endl; // Debug
                options.headersOnly = true;
                break;
            case 'a':
                std::cout << "Option -a with value " << optarg << std::endl; // Debug
                options.authFile = optarg;
                break;
            case 'b':
                std::cout << "Option -b with value " << optarg << std::endl; // Debug
                options.mailbox = optarg;
                break;
            case 'o':
                std::cout << "Option -o with value " << optarg << std::endl; // Debug
                options.outputDir = optarg;
                break;
            default:
                printUsage();
                std::exit(EXIT_FAILURE);
        }
    }

    // Get the server argument
    if (optind < argc) {
        options.server = argv[optind];
    } else {
        std::cerr << "Adresa serveru je povinná." << std::endl;
        printUsage();
        exit(EXIT_FAILURE);
    }

    // Check for required options
    if (options.authFile.empty() || options.outputDir.empty()) {
        std::cout << options.authFile << " " << options.outputDir << std::endl;
        std::cerr << "Argumenty -a a -o jsou povinná." << std::endl;
        printUsage();
        exit(EXIT_FAILURE);
    }


    // Set default port based on TLS usage
    if (options.port == -1) {
        options.port = options.useTLS ? 993 : 143;
    }

    return options;
}

void ArgumentsParser::printUsage() {
    std::cout << std::endl << "Použití: imapcl <adresa_serveru> [options]" << std::endl;
    std::cout << "Povinné argumenty:" << std::endl;
    std::cout << "  <adresa_serveru>         Adresa IMAP serveru" << std::endl;
    std::cout << "  -a <auth_soubor>         Cesta k autentizačnímu souboru" << std::endl;
    std::cout << "  -o <výstupní_adresář>    Cesta k adresáři pro ukládání emailů" << std::endl;
    std::cout << std::endl;
    std::cout << "Volitelné argumenty:" << std::endl;
    std::cout << "  -p <port>                Specifikujte číslo portu" << std::endl;
    std::cout << "  -T                       Použít protokol TLS" << std::endl;
    std::cout << "    -c <cert_soubor>       Cesta k souboru s certifikátem" << std::endl;
    std::cout << "    -C <cert_adresář>      Cesta k adresáři s certifikáty (defaultně /etc/ssl/certs)" << std::endl;
    std::cout << "  -n                       Stáhnout pouze nové zprávy" << std::endl;
    std::cout << "  -h                       Stáhnout pouze hlavičky zpráv" << std::endl;
    std::cout << "  -b <schránka>            Název schránky (výchozí je INBOX)" << std::endl;
}