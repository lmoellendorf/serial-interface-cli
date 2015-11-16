#ifndef SF_STRINGHEX_H
#define SF_STRINGHEX_H

#include <string>
#include <stdint.h>
#include <vector>

class StringHex
{
public:
    StringHex();
    ~StringHex();

    std::vector<uint8_t> &HexStringToBinary ( std::string &hex_string );
    std::string *BinaryToHexString(uint8_t *binaries, size_t length);
};

#endif // SF_STRINGHEX_H
