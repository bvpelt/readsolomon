// reed.cpp
#include <iostream>
#include <string>
#include "rs_codec.hpp"

int main(int argc, char* argv[])
{
    if (argc != 3) {
        std::cerr << "Usage:\n"
                  << "  reed -e <filename>   # Encode file → filename.parity\n"
                  << "  reed -d <filename>   # Decode filename.parity → filename.recovered\n";
        return 1;
    }

    std::string option = argv[1];
    std::string filename = argv[2];

    if (option == "-e") {
        if (!rs_encode_file(filename)) {
            std::cerr << "Encoding failed.\n";
            return 1;
        }
    } else if (option == "-d") {
        if (!rs_decode_file(filename)) {
            std::cerr << "Decoding failed.\n";
            return 1;
        }
    } else {
        std::cerr << "Unknown option: " << option << "\n";
        return 1;
    }

    return 0;
}
