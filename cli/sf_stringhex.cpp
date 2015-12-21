#include "sf_stringhex.h"
#include <string.h>
#include <stdio.h>
#include <bitset>

namespace sf
{

StringHex::StringHex ( std::string delimiter )
{
  if ( !delimiter.empty() )
    {
      this->delimiter = delimiter;
    }
}

StringHex::StringHex( )
{

}

StringHex::~StringHex()
{

}

std::vector<uint8_t> &StringHex::HexStringToBinary ( std::string &hex_string,
    std::vector<uint8_t> &hex_binaries )
{

  /** Needed for tokenizing string at spaces */
  char *saveptr = NULL;
  char *token;
  char *c_hex_string;
  char *pass_to_strtok_r;

  size_t hex_string_length;
  size_t token_length;
  char hex_array[] = { 0, 0, 0};
  unsigned long byte;
  char *endptr;

  hex_string_length = hex_string.length();
  /** Copy the whole string for tokenizing */
  c_hex_string = ( char* ) std::malloc ( hex_string_length + 1 );
  strncpy ( c_hex_string, hex_string.c_str(), hex_string_length + 1 );
  /* Copy the pointer so the tokenizer may set it to NULL */
  pass_to_strtok_r = c_hex_string;

  /** sigh! "warning: binary constants are a C++14 feature or GCC extension"
  const uint8_t case_mask = 0b00100000;
  */
  std::bitset<8> case_mask( "00100000" ) ; // construct from std::string

  do
    {
      token = strtok_r ( pass_to_strtok_r, delimiter.c_str(), &saveptr );
      /** Set to NULL for all consecutive iterations */
      pass_to_strtok_r = NULL;
      if ( token )
        {
          for ( size_t i = 0; i <
                ( token_length = strnlen ( token, hex_string_length ) ); i+=2 )
            {
              /* Clean the array in case only one character is left */
              memset ( hex_array, 0, sizeof hex_array );
              /**
               * Ignore "0x" and "0X"
               * Because strtoul gets only 2 byte to see, passing
               * hexadecimal prefixes is not possible here.
               */
              if ( token_length > i + 1  && '0' == * ( token + i ) &&
                   ( case_mask |= * ( token + i + 1 ) ) == 'x' )
                {
                  continue;
                }
              * ( ( uint16_t* ) hex_array ) =
                * ( ( uint16_t* ) ( token + i ) );
              byte = strtoul ( hex_array, &endptr, 16 );
              /* Check if the whole string had been valid */
              if ( '\0' != *hex_array && '\0' == *endptr )
                {
                  hex_binaries.push_back ( byte );
                }
              else
                {
                  /* Stop at the first invalid character */
                  break;
                }
            }
        }
    }
  while ( token );

  return hex_binaries;
}

std::string &StringHex::BinaryToHexString ( std::vector<uint8_t> &hex_binaries,
                                            std::string &hex_string )
{
  const size_t size = 3;
  char str[size];

  for ( std::vector<uint8_t>::iterator hex_binary = hex_binaries.begin() ;
        hex_binary != hex_binaries.end();
        ++hex_binary )
    {
      if ( (int) (size - 1) == std::snprintf ( str, size, "%2.2X", *hex_binary ) )
        {
          hex_string += str;
          hex_string += " ";
        }
    }
    /**
     * To avoid exceptions check for emptiness first, then check if the loop
     * above has been run in which case one ' ' has been appended too much.
     */
    if(!hex_string.empty() && ' ' == hex_string.back()){
      /** Remove the trailing ' '. */
      hex_string.pop_back();
    }

  return hex_string;
}
}
