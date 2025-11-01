// rs_codec.cpp
#include "rs_codec.hpp"
#include <fstream>
#include <iostream>
#include <vector>
#include <cstdint>
#include "ezpwd/rs"   // adjust include path if needed

// We'll use RS(255,223): 32 bytes parity
static ezpwd::RS<255,223> rs;

// Encode ---------------------------------------------------
bool rs_encode_file(const std::string& inputFile)
{
    std::ifstream in(inputFile, std::ios::binary);
    if (!in) {
        std::cerr << "Cannot open input file: " << inputFile << "\n";
        return false;
    }

    /*
    // less data
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(in)),
                              std::istreambuf_iterator<char>());

    const size_t payload = rs.load();
    std::vector<uint8_t> parity;
    std::vector<uint8_t> parityAll;

    for (size_t offset = 0; offset < data.size(); offset += payload) {
        size_t chunkLen = std::min(payload, data.size() - offset);
        std::vector<uint8_t> chunk(data.begin() + offset,
                               data.begin() + offset + chunkLen);

        rs.encode(chunk, parity);

        // Build full codeword
        std::vector<uint8_t> codeword = chunk;
        codeword.insert(codeword.end(), parity.begin(), parity.end());

        parityAll.insert(parityAll.end(), codeword.begin(), codeword.end());
    }
    */

    /*
    // write parity
    std::string outFile = inputFile + ".parity";
    std::ofstream out(outFile, std::ios::binary);
    if (!out) {
        std::cerr << "Cannot open output: " << outFile << "\n";
        return false;
    }
    out.write(reinterpret_cast<char*>(parityAll.data()), parityAll.size());
    out.close();
    */

    // Lees hele bestand in
    std::vector<uint8_t> filebuf((std::istreambuf_iterator<char>(in)),
                                  std::istreambuf_iterator<char>());

    // Kies RS parameters RS(255, 223) -> 32 parity bytes (kan tot 16 symbol errors herstellen)
    // Voor 8-bit symbolen SIZE=255, PAYLOAD=223 => 32 parity
    ezpwd::RS<255,223> rs;

    // Encode: rs.encode(filebuf, parity) of rs.encode inplace (voegt parity bytes aan vector toe)
    // We moeten filechunks maken van max payload=rs.payload()
    const size_t payload = rs.load();
    std::vector<uint8_t> parity;
    std::vector<uint8_t> out_all; // hier schrijven we parity-sidecar op blok-basis

    for (size_t offset = 0; offset < filebuf.size(); offset += payload) {
        size_t chunklen = std::min(payload, filebuf.size() - offset);
        std::vector<uint8_t> chunk(filebuf.begin() + offset, filebuf.begin() + offset + chunklen);
        // Als chunk kleiner is dan payload, pad met nulls - rs.encode accepteert kortere chunks
        rs.encode(chunk, parity); // parity resized to rs.nroots()
        // Sla parity op (sidecar): eerst chunklen (4 bytes) dan parity
        uint32_t l = (uint32_t)chunklen;
        out_all.insert(out_all.end(), (uint8_t*)&l, (uint8_t*)&l + sizeof(l));
        out_all.insert(out_all.end(), parity.begin(), parity.end());
    }

     // schrijf parity-sidecar
    std::string outFile = inputFile + ".parity";
    std::ofstream pout(outFile, std::ios::binary);
    pout.write((char*)out_all.data(), out_all.size());
    pout.close();
    //std::cout << "Parity geschreven naar " << outFile << "\n";
    
    std::cout << "Encoded " << inputFile << " → " << outFile << "\n";
    return true;
}

// Decode ---------------------------------------------------
bool rs_decode_file(const std::string& inputFile)
{
    // inputFile should have .parity extension
    std::string baseFile = inputFile;
    if (baseFile.size() > 7 &&
        baseFile.substr(baseFile.size() - 7) == ".parity") {
        baseFile = baseFile.substr(0, baseFile.size() - 7);
    }

    /*
    std::ifstream dataIn(baseFile, std::ios::binary);
    std::ifstream parityIn(inputFile, std::ios::binary);
    if (!dataIn || !parityIn) {
        std::cerr << "Cannot open input or parity file.\n";
        return false;
    }

    std::vector<uint8_t> data((std::istreambuf_iterator<char>(dataIn)),
                              std::istreambuf_iterator<char>());
    std::vector<uint8_t> parityAll((std::istreambuf_iterator<char>(parityIn)),
                                   std::istreambuf_iterator<char>());

    //size_t payload = rs.load();
    size_t offset = 0;
    size_t parityOffset = 0;
    std::vector<uint8_t> recovered;

    while (parityOffset + sizeof(uint32_t) <= parityAll.size()) {
        uint32_t chunkLen;
        std::memcpy(&chunkLen, &parityAll[parityOffset], sizeof(uint32_t));
        parityOffset += sizeof(uint32_t);
        if (parityOffset + rs.nroots() > parityAll.size()) break;

        std::vector<uint8_t> parity(parityAll.begin() + parityOffset,
                                    parityAll.begin() + parityOffset + rs.nroots());
        parityOffset += rs.nroots();

        if (offset >= data.size()) break;
        size_t chunkSize = std::min((size_t)chunkLen, data.size() - offset);
        std::vector<uint8_t> chunk(data.begin() + offset,
                                   data.begin() + offset + chunkSize);

        // Simulate potential damage repair:
        bool ok = rs.decode(chunk, parity);
        if (!ok) {
            std::cerr << "Warning: chunk decode failed at offset " << offset << "\n";
        }
        recovered.insert(recovered.end(), chunk.begin(), chunk.end());
        offset += chunkLen;
    }

    std::string outFile = baseFile + ".recovered";
    std::ofstream out(outFile, std::ios::binary);
    out.write(reinterpret_cast<char*>(recovered.data()), recovered.size());
    out.close();
    */


    // === Load corrupted file (or original if you want to test no errors) ===
    std::ifstream in(inputFile, std::ios::binary);
    if (!in) { std::cerr << "Kon input niet openen\n"; return 1; }
    std::vector<uint8_t> filebuf((std::istreambuf_iterator<char>(in)),
                                 std::istreambuf_iterator<char>());
    in.close();

    // === Load parity sidecar ===
    std::string parityFile = inputFile + ".parity";
    std::ifstream pin(parityFile, std::ios::binary);
    if (!pin) { std::cerr << "Kon parity niet openen\n"; return 1; }
    std::vector<uint8_t> parbuf((std::istreambuf_iterator<char>(pin)),
                                 std::istreambuf_iterator<char>());
    pin.close();

    // === Setup Reed–Solomon ===
    ezpwd::RS<255,223> rs;
    // begin change
    // const size_t K = ezpwd::RS<255,223>::kk;
    // These are the template parameters: 255 total symbols, 223 data bytes.
    //const size_t N = 255;
    const size_t K = 223;
    //const size_t parity_len = rs.nroots();
    // end change

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

    std::string outFile = baseFile + ".recovered";
    std::cout << "outFile: " << outFile << "\n";
    std::ofstream out(outFile, std::ios::binary);
    out.write(reinterpret_cast<char*>(repaired.data()), repaired.size());
    out.close();

    std::cout << "Decoded to " << outFile << "\n";
    return true;
}
