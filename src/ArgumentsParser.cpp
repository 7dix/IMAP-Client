// ArgumentsParser.cpp
#include "ArgumentsParser.h"
#include <iostream>
#include <unistd.h>
#include <cstdlib>


ProgramOptions ArgumentsParser::parse(int argc, char* argv[]) {
    ProgramOptions options;

    if (argc < 2) {
        printUsage();
        throw std::invalid_argument("Nebyly zadány žádné argumenty.");
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
                    throw std::invalid_argument("Číslo portu je mimo rozsah (1-65535)");
                }
                options.port = std::atoi(optarg);
                break;
            case 'T':
                options.useTLS = true;
                break;
            case 'c':
                if (!optarg || !options.useTLS) {
                    printUsage();
                    throw std::invalid_argument("Chybí argument -T pro použití TLS.");
                }
                options.certFile = optarg;
                break;
            case 'C':
                if (!options.useTLS) {
                    printUsage();
                    throw std::invalid_argument("Chybí argument -T pro použití TLS.");
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
                throw std::invalid_argument("Neznámý argument.");
        }
    }

    // Check for required options after option parsing
    if (options.authFile.empty()) {
        printUsage();
        throw std::invalid_argument("Chybí povinný argument -a.");
    }
    if (options.outputDir.empty()) {
        printUsage();
        throw std::invalid_argument("Chybí povinný argument -o.");
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