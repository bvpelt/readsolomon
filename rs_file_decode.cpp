// rs_file_decode.cpp
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include "ezpwd/rs"

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Gebruik: " << argv[0] << " input_corrupt.txt file.parity\n";
        return 1;
    }
    std::string infile = argv[1];
    std::string parityfile = argv[2];

    // === Load corrupted file (or original if you want to test no errors) ===
    std::ifstream in(infile, std::ios::binary);
    if (!in) { std::cerr << "Kon input niet openen\n"; return 1; }
    std::vector<uint8_t> filebuf((std::istreambuf_iterator<char>(in)),
                                 std::istreambuf_iterator<char>());
    in.close();

    // === Load parity sidecar ===
    std::ifstream pin(parityfile, std::ios::binary);
    if (!pin) { std::cerr << "Kon parity niet openen\n"; return 1; }
    std::vector<uint8_t> parbuf((std::istreambuf_iterator<char>(pin)),
                                 std::istreambuf_iterator<char>());
    pin.close();

    // === Setup Reed–Solomon ===
    ezpwd::RS<255,223> rs;
    const size_t K = 223;

    const size_t parity_len = rs.nroots();

    std::vector<uint8_t> repaired;
    size_t pos = 0, offset = 0;

    while (pos + sizeof(uint32_t) + parity_len <= parbuf.size() && offset < filebuf.size()) {
        uint32_t chunklen;
        std::memcpy(&chunklen, &parbuf[pos], sizeof(uint32_t));
        pos += sizeof(uint32_t);

        std::vector<uint8_t> parity(parbuf.begin() + pos, parbuf.begin() + pos + parity_len);
        pos += parity_len;

        size_t actual_chunk = std::min<size_t>(chunklen, filebuf.size() - offset);
        std::vector<uint8_t> chunk(filebuf.begin() + offset,
                                   filebuf.begin() + offset + actual_chunk);

        // Simulate corruption (for testing)
        if (!chunk.empty() && (offset / K) % 2 == 0) {
            if (chunk.size() > 5) {
                chunk[1] ^= 0xFF;
                chunk[5] ^= 0xFF;
                std::cout << "Simulated corruption in block " << (offset / K) << "\n";
            }
        }

        // Attempt to repair in place
        bool ok = rs.decode(chunk, parity);
        if (!ok) {
            std::cerr << "Block " << (offset / K) << " could not be repaired!\n";
        } else {
            std::cout << "Block " << (offset / K) << " repaired successfully.\n";
        }

        repaired.insert(repaired.end(), chunk.begin(), chunk.begin() + chunklen);
        offset += chunklen;
    }

    std::ofstream out("recovered.txt", std::ios::binary);
    out.write(reinterpret_cast<char*>(repaired.data()), repaired.size());
    out.close();

    std::cout << "Repaired file written to recovered.txt\n";
    return 0;
}
