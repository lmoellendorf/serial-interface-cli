/*! ============================================================================
 *
 * @file main.cpp
 *
 * @date: 20.11.2014
 * @author: © by STACKFORCE, Heitersheim, Germany, http://www.stackforce.de
 * @author: Lars Möllendorf
 *
 * @brief  Sample header of the source code
 *
 * @version
 *
 =============================================================================*/

#include <iostream>
#include <getopt.h>
#include <libgen.h>

#include "sf_subject.h"

extern "C"
{
#include <libserialport.h>
}

#include "sf_serialmac.h"
#include "version.h"
#include "sf_serialmaccli.h"

static int usage ( int exitCode, char *commandPath );

/* allowed short and long options */
static const char *shortOptString = "p:idh";
static const struct option longOptString[] =
{
  {"port", required_argument,      NULL, 'p'},
  {"interactive", no_argument,      NULL, 'i'},
  {"debug",    optional_argument,      NULL, 'd'},
  {"help",    no_argument,            NULL, 'h'},
  {0, 0, 0, 0}
};

static int usage ( int exitCode, char *commandPath )
{
  std::string companyName = SERIALMAC_PRODUCT_COMPANY;
  std::string productName = SERIALMACCLI_PRODUCT_NAME;
  std::string protocolName = SERIALMAC_PRODUCT_NAME;
  char *commandName = basename ( commandPath );
  std::ostream *out = &std::cout;

  if ( 0 != exitCode )
    {
      out = &std::cerr;
    }

  *out << "Usage: "
       << commandName
       << " [OPTIONS]\n"
       << companyName
       << " "
       << productName
       << "\n"
       << "Send and receive "
       << companyName
       << " "
       << protocolName
       << " frames via the serial interface.\n\n"
       << "Default parameters:\n\n"
       << "\tRunning "
       << commandName
       << " without parameters will use the following default values:\n\n"
       << "\tserial port : The first available serial port.\n"
       << "Optional arguments:\n\n"
       << "\t-p\t\tSerial port.\n\n"
       << "\t-d\t\tDebug information on stderr.\n"
       << "\t-h\t\tThis helpful blurb.\n"
       << "Exit status:\n\n"
       << "\t0 if OK\n"
       << "\t1 if error\n"
       << std::endl;
  exit ( exitCode );
}

int main ( int argc, char **argv )
{

  char *portname = NULL;
  SerialMacCli serialmac();

  int opt = 0;
  int long_index = 0;

  while ( ( opt = getopt_long ( argc, argv, shortOptString, longOptString, &long_index ) ) != -1 )
    {
      switch ( opt )
        {
        case 'p':
          portname=optarg;
          break;

        case 'i':

          break;

        case 'd':
          break;

        case 'h':
          usage ( 0, argv[0] );
          break;
        case '?':
          usage ( 1, argv[0] );
          break;

        default:
          /* You won't actually get here. */
          std::cerr << "Error: This shouldn't have happened!" << std::endl;
          break;
        }
    }

  SerialMacCli cli ( portname );

  while ( 1 );

  return 0;
}
