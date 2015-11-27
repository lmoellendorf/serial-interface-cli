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
  unsigned long result;
  char *endptr;
  for ( size_t i = 0; i < hex_string.length(); i+=2 )
    {
      /* Check if there follows another character */
      if ( hex_string.length() >= i + 1 )
        {
          * ( ( uint16_t* ) hex_array ) = * ( ( uint16_t* ) ( hex_string.c_str() + i ) );
          result = strtoul ( hex_array, &endptr, 16 );
          /* Check if the whole string had been valid */
          if ( '\0' != *hex_array && '\0' == *endptr )
            {
              hex_binaries.push_back ( result );
            }
        }
    }

  return hex_binaries;
}
