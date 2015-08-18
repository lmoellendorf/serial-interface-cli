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
#include <stdio.h>
#include <thread>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <libgen.h>

extern "C"
{
#include <libserialport.h>
}

#include "sf_serialmac.h"
#include "version.h"

#define SF_SERIAL_BAUDRATE 115200
#define SF_SERIAL_BITS 8
#define SF_SERIAL_STOPBITS 1
#define SF_SERIAL_FLOWCTRL SP_FLOWCONTROL_NONE
#define SF_SERIAL_INPUT_MAX_SIZE 255
#define FALSE 0
#define TRUE 1

using namespace std;

enum state
{
  START_FRAME,
  APPEND_FRAME,
};

struct app_ctx
{
  int run = TRUE;
  int status = START_FRAME;
  struct sp_port *port = NULL;
  struct sf_serialmac_ctx *mac_ctx;
  size_t iBuffLen = 0;
  size_t oBuffRemains = 0;
  size_t oBuffLength = 0;
  char iBuff[SF_SERIAL_INPUT_MAX_SIZE];
  char oBuff[SF_SERIAL_INPUT_MAX_SIZE];
};

static struct app_ctx ctx;


void read_evt ( const char *frameBuffer, size_t frameBufferLength );
void bufferRx_evt ( const char *frameBuffer, size_t frameBufferLength );
void write_evt ( size_t processed );
void bufferTx_evt ( int processed );
void wait4userinput ( void );
void wait4halEvent ( enum sp_event event,
                     enum sf_serialmac_return ( *sf_serialmac_halCb ) ( struct sf_serialmac_ctx *ctx ) );
void wait4halTxEvent();
void wait4halRxEvent();
static int usage ( int exitCode, char *commandPath );


void read_evt ( const char *frameBuffer, size_t frameBufferLength )
{
  /**
   * FIXME: Be prepared: the MAC should also call this, when a frame has been
   * rejected.
   */
  if ( frameBuffer && frameBufferLength )
    {
      if ( '\n' == frameBuffer[0] )
        {
          ctx.run = FALSE;
        }
      else
        {
          printf ( ":%s:%zd\n", frameBuffer, frameBufferLength );
        }
    }
}

void bufferRx_evt ( const char *frameBuffer, size_t frameBufferLength )
{
  sf_serialmac_rx_frame ( ( struct sf_serialmac_ctx * ) ctx.mac_ctx, ctx.iBuff,
                          sizeof ( ctx.iBuff ) );
}

void write_evt ( size_t processed )
{
  ctx.status = START_FRAME;
}

void bufferTx_evt ( size_t processed )
{
  ctx.oBuffRemains -= processed;
  thread userInputEventLoop ( wait4userinput );
  userInputEventLoop.detach();
}

void wait4userinput ( void )
{
  enum sf_serialmac_return ret = SF_SERIALMAC_SUCCESS;
  string line = "";
  const size_t frmLength = 9;

  if ( !ctx.oBuffRemains )
    {
      printf ( "Input text:\n" );
      getline ( cin, line );
      if ( line.length() > 0 )
        {
          ctx.oBuffLength = ctx.oBuffRemains = line.length();
          strncpy ( ctx.oBuff, line.c_str(), sizeof ctx.oBuff );
        }
      else
        {
          /** Userinput was empty line -> STOP */
          ctx.run = FALSE;
          printf ( "Stop\n" );
        }
    }
  if ( ctx.oBuffRemains )
    {
      while ( ( ret = sf_serialmac_tx_frame ( ctx.mac_ctx, frmLength,
                                              ctx.oBuff + ( ctx.oBuffLength - ctx.oBuffRemains ),
                                              ctx.oBuffRemains ) ) != SF_SERIALMAC_SUCCESS )
        {
          cerr << "TX Error " << ret << "\nline: "
          << ctx.oBuff + ( ctx.oBuffLength - ctx.oBuffRemains )
          << "\nlength: "
          << ctx.oBuffRemains
          << endl;
          sleep ( 1 );
        }
      //        switch (ctx.status)
//        {
//        case START_FRAME:
//            if((ret = sf_serialmac_txFrameStart(ctx.mac_ctx,
//                                                 frmLength)) != SF_SERIALMAC_SUCCESS)
//            {
//                printf("Frame Error %i\n", ret);
//            }
//            ctx.status = APPEND_FRAME;
//        //break; omitted
//        case APPEND_FRAME:
//            while((ret = sf_serialmac_txFrameAppend(ctx.mac_ctx,
//                         ctx.oBuff + (ctx.oBuffLength - ctx.oBuffRemains),
//                         ctx.oBuffRemains)) != SF_SERIALMAC_SUCCESS)
//            {
//                printf("TX Error %i\nline: %s\nlength: %zd\n", ret,
//                       ctx.oBuff + (ctx.oBuffLength - ctx.oBuffRemains), ctx.oBuffRemains);
//                sleep(1);
//            }
//            break;
//        default:
//            printf("Exception Error\n");
//            break;
//        }
    }
}

void wait4halTxEvent()
{
  wait4halEvent ( SP_EVENT_TX_READY, sf_serialmac_hal_tx_callback );
}

void wait4halRxEvent()
{
  wait4halEvent ( SP_EVENT_RX_READY, sf_serialmac_hal_rx_callback );
}
/**
 * This is a method stub to react on libserialport events. The plan is to
 * create 2 APIs for the MAC:
 * <ul>
 * <li>event driven</li>
 * <li>active polling</li>
 * </ul>
 */
void wait4halEvent ( enum sp_event event,
                     enum sf_serialmac_return ( *sf_serialmac_halCb ) ( struct sf_serialmac_ctx *ctx ) )
{
  struct sp_event_set *portEventSet = NULL;
  unsigned int portEventMask = event;
  unsigned int timeout = 0;

  if ( SP_OK <= sp_new_event_set ( &portEventSet ) )
    {

      if ( SP_OK
           <= sp_add_port_events ( portEventSet, ctx.port,
                                   ( enum sp_event ) portEventMask ) )
        {
          while ( SP_OK <= sp_wait ( portEventSet, timeout ) )
            {
              sf_serialmac_halCb ( ctx.mac_ctx );
              sleep ( 1 );
            }
        }
      sp_free_event_set ( portEventSet );
    }
  return;
}

/* allowed short and long options */
static const char *shortOptString = "p:dh";
static const struct option longOptString[] =
{
  {"port", required_argument,      NULL, 'p'},
  {"debug",    optional_argument,      NULL, 'd'},
  {"help",    no_argument,            NULL, 'h'},
  {0, 0, 0, 0}
};

static int usage ( int exitCode, char *commandPath )
{
  char *companyName = SERIALMAC_PRODUCT_COMPANY;
  char *productName = SERIALMACCLI_PRODUCT_NAME;
  char *protocolName = SERIALMAC_PRODUCT_NAME;
  char *commandName = basename ( commandPath );
  ostream *out = &cout;

  if ( 0 != exitCode )
    {
      out = &cerr;
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
  << endl;
  exit ( exitCode );
}

int main ( int argc, char **argv )
{

  uint8_t mac_ctx[sf_serialmac_ctx_size()];
  ctx.mac_ctx = ( struct sf_serialmac_ctx * ) mac_ctx;

  sp_return sp_ret = SP_OK;
  char *portname = NULL;
  struct sp_port **availablePorts = NULL;
  struct sp_port_config *savedPortConfig = NULL;

  int opt = 0;
  int long_index = 0;

  while ( ( opt = getopt_long ( argc, argv, shortOptString, longOptString, &long_index ) ) != -1 )
    {
      switch ( opt )
        {
        case 'p':
          portname=optarg;
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
          cerr << "Error: This shouldn't have happened!" << endl;
          break;
        }
    }

  /** If the user specified no port, choose any. */
  if ( NULL == portname )
    {
      sp_ret = sp_list_ports ( &availablePorts );
      if ( SP_OK > sp_ret )
        {
          usage ( sp_ret, argv[0] );
        }
      if ( NULL != availablePorts[0] )
        {
          sp_ret = sp_copy_port ( availablePorts[0], &ctx.port );
        }
      if ( NULL != availablePorts )
        {
          sp_free_port_list ( availablePorts );
        }
      if ( SP_OK > sp_ret  || NULL == ctx.port )
        {
          cerr << "Could not find any serial port!\n" << endl;
          usage ( (0 == sp_ret ? 1 : sp_ret), argv[0] );
        }
    }
  else
    {
      sp_ret = sp_get_port_by_name ( portname, &ctx.port );
      if ( SP_OK > sp_ret || NULL == ctx.port )
        {
          cerr << "Port \"" << portname << "\" could not be found!\n" << endl;
          usage ( (0 == sp_ret ? 1 : sp_ret), argv[0] );
        }
    }

  sp_ret = sp_open ( ctx.port, SP_MODE_READ_WRITE );
  if ( SP_OK > sp_ret )
    {
      cerr << "Port \"" << portname << "\" could not be opened!\n" << endl;
      usage ( sp_ret, argv[0] );
    }

  /** Save current port configuration for later restoring */
  sp_ret = sp_new_config ( &savedPortConfig );
  if ( SP_OK > sp_ret )
    {
      cerr << "Config of port  \"" << portname << "\" could not be saved! (Out of memory?)\n" << endl;
      usage ( sp_ret, argv[0] );
    }
  sp_ret = sp_get_config ( ctx.port, savedPortConfig );
  if ( SP_OK > sp_ret )
    {
      cerr << "Config of port  \"" << portname << "\" could not be saved! (Read error?)\n" << endl;
      usage ( sp_ret, argv[0] );
    }

  sp_ret = sp_set_baudrate ( ctx.port, SF_SERIAL_BAUDRATE );
  if ( SP_OK > sp_ret )
    {
      cerr << "Could not set baudrate to " << SF_SERIAL_BAUDRATE << " on port \"" << portname << "\"!\n" << endl;
      usage ( sp_ret, argv[0] );
    }

  sp_ret = sp_set_bits ( ctx.port, SF_SERIAL_BITS );
  if ( SP_OK > sp_ret )
    {
      cerr << "Could not set number of bits to " << SF_SERIAL_BITS << " on port \"" << portname << "\"!\n" << endl;
      usage ( sp_ret, argv[0] );
    }

  sp_ret = sp_set_parity ( ctx.port, SP_PARITY_NONE );
  if ( SP_OK > sp_ret )
    {
      cerr << "Could not set parity to " << SP_PARITY_NONE << " on port \"" << portname << "\"!\n" << endl;
      usage ( sp_ret, argv[0] );
    }

  sp_ret = sp_set_stopbits ( ctx.port, SF_SERIAL_STOPBITS );
  if ( SP_OK > sp_ret )
    {
      cerr << "Could not set stop-bits to " << SF_SERIAL_STOPBITS << " on port \"" << portname << "\"!\n" << endl;
      usage ( sp_ret, argv[0] );
    }

  sp_ret = sp_set_flowcontrol ( ctx.port, SF_SERIAL_FLOWCTRL );
  if ( SP_OK > sp_ret )
    {
      cerr << "Could not set flow-control to " << SF_SERIAL_FLOWCTRL << " on port \"" << portname << "\"!\n" << endl;
      usage ( sp_ret, argv[0] );
    }

  sf_serialmac_init ( ctx.mac_ctx,
                      ( void * ) ctx.port,
                      ( SF_SERIALMAC_HAL_READ_FUNCTION ) sp_nonblocking_read,
                      ( SF_SERIALMAC_HAL_READ_WAIT_FUNCTION ) sp_input_waiting,
                      ( SF_SERIALMAC_HAL_WRITE_FUNCTION ) sp_nonblocking_write,
                      read_evt, bufferRx_evt,
                      write_evt, bufferTx_evt );

  /** Start waiting for user input */
  thread userInputEventLoop ( wait4userinput );
  userInputEventLoop.detach();

  /* Loop until the user quits */
  while ( ctx.run )
    {
      sleep ( 1 );
      sf_serialmac_entry ( ( struct sf_serialmac_ctx * ) ctx.mac_ctx );
    }

  if ( NULL != ctx.port )
    {
      /** Restore previous port configuration */
      sp_ret = sp_set_config ( ctx.port, savedPortConfig );
      sp_free_port ( ctx.port );
    }
  return sp_ret;
}
