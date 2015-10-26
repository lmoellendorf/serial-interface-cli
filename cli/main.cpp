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

#include "sf_serialmaccli.h"

int main ( int argc, char **argv )
{

  SerialMacCli cli;

  //TODO: error handling
  cli.Init( argc, argv );
  return cli.Run();
}
