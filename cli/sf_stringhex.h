#ifndef SF_STRINGHEX_H
#define SF_STRINGHEX_H

#include <string>
#include <stdint.h>
#include <vector>

class StringHex
{
public:
    StringHex(std::string delimiter);
    StringHex();
    ~StringHex();

    std::vector<uint8_t> &HexStringToBinary ( std::string &hex_string,
            std::vector<uint8_t> &hex_binaries );
    std::string &BinaryToHexString ( std::vector<uint8_t> &hex_binaries,
                                     std::string &hex_string );
private:
    std::string delimiter = " ,;.:";
};

#endif // SF_STRINGHEX_H
