// reed.cpp
#include <iostream>
#include <string>
#include "rs_codec.hpp"

int main(int argc, char* argv[])
{
    if ((argc != 3) && (argc != 5)) {
        std::cerr << "Usage:\n"
                  << "  reed -e <inputfile>                     # Encode → <inputfile>.parity\n"
                  << "  reed -d <inputfile>                     # Decode using <inputfile>.parity\n"
                  << "  reed -d <inputfile> -p <parityfile>     # Decode using custom parity file\n";
         return 1;
    }

    std::string option = argv[1];
    std::string inputFile = argv[2];
    std::string parityFile;
    
    // Optional parity file argument for decoding
    if (option == "-d" && argc >= 5) {
        if (std::string(argv[3]) == "-p") {
            parityFile = argv[4];
        } else {
            std::cerr << "Unknown option after -d: " << argv[3] << "\n";
            return 1;
        }
    }

    if (option == "-e") {
        if (!rs_encode_file(inputFile)) {
            std::cerr << "Encoding failed.\n";
            return 1;
        }
    } 
    else if (option == "-d") {
        if (!rs_decode_file(inputFile, parityFile)) {
            std::cerr << "Decoding failed.\n";
            return 1;
        }
    } 
    else {
        std::cerr << "Unknown option: " << option << "\n";
        return 1;
    }
    return 0;
}
