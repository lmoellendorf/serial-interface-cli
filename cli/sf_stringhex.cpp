#include "sf_stringhex.h"
#include <string.h>

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

  /** Needed for tokenizing string at spaces */
  const char *delim = " ";
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

  do
    {
      token = strtok_r ( pass_to_strtok_r, delim, &saveptr );
      /** Set to NULL for all consecutive iterations */
      pass_to_strtok_r = NULL;
      if ( token )
        {
          for ( size_t i = 0; i <
                ( token_length = strnlen ( token, hex_string_length ) ); i+=2 )
            {
              /* Check if there follows another character */
              if ( token_length >= i + 1 )
                {
                  /**
                   * Ignore "0x"
                   * Because strtoul gets only 2 byte to see, passing
                   * hexadecimal prefixes is not possible here.
                   */
                  if ( '0' == * ( token + i ) &&
                    'x' ==  * ( token + i + 1 ) )
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
    }
  while ( token );

  return hex_binaries;
}
