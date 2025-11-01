// rs_file_example.cpp
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include "ezpwd/rs"   // pad naar header in je include path

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Gebruik: " << argv[0] << " input.txt\n";
        return 1;
    }
    std::string infile = argv[1];
    std::ifstream in(infile, std::ios::binary);
    if (!in) { std::cerr << "Kon input niet openen\n"; return 1; }

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
    std::ofstream pout("file.parity", std::ios::binary);
    pout.write((char*)out_all.data(), out_all.size());
    pout.close();
    std::cout << "Parity geschreven naar file.parity\n";

    // --- Simuleer fouten: nu laten we een chunk corrupt gaan en proberen te herstellen ---
    // (In praxis: je zou file + parity gebruiken; hier demonstratief)
    // Voor decoding: lees sidecar en roep decode op elk chunk met data+parity (ezpwd supports decode in-place)
    // (Deze decode flow vereist je de oorspronkelijke chunk en parity samen te vormen; als data beschadigd of weg is,
    //  je kunt parity+overige informatie gebruiken om herstellen.)

    return 0;
}
