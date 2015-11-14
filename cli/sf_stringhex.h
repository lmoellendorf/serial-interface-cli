#ifndef SF_STRINGHEX_H
#define SF_STRINGHEX_H

#include <string>
#include <stdint.h>

class StringHex
{
public:
    StringHex();
    ~StringHex();

    size_t HexStringToBinary(std::string *hex_string, uint8_t **hex_array);
    std::string *BinaryToHexString(uint8_t *binaries, size_t length);
};

#endif // SF_STRINGHEX_H
