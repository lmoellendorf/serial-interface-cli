#include "sf_stringhex.h"

StringHex::StringHex()
{

}

StringHex::~StringHex()
{

}

std::string *StringHex::BinaryToHexString ( uint8_t* binaries, size_t length )
{
  return NULL;
}

std::vector<uint8_t> &StringHex::HexStringToBinary ( std::string &hex_string,
    std::vector<uint8_t> &hex_binaries )
{

  char hex_array[] = { 0, 0, 0};
  for ( size_t i = 0; i < hex_string.length(); i+=2 )
    {
      /* Check if there follows another character */
      if ( hex_string.length() >= i + 1 )
        {
          * ( ( uint16_t* ) hex_array ) = * ( ( uint16_t* ) ( hex_string.c_str() + i ) );
          hex_binaries.push_back ( strtoul ( hex_array,NULL,16 ) );
        }
    }

  return hex_binaries;
}
