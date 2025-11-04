// rs_codec.hpp
#ifndef RS_CODEC_HPP
#define RS_CODEC_HPP

#include <string>

bool rs_encode_file(const std::string& inputFile);
bool rs_decode_file(const std::string& inputFile,
                    const std::string& parityFile = "");

#endif
