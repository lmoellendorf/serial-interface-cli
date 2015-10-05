#include <iostream>
#include <thread>
#include <string.h>
#include <docopt.h>

#include "sf_serialmaccli.h"
#include "sf_serialmachandler.h"
#include "sf_serialmac.h"
#include "version.h"


    static const char USAGE[] =
      SERIALMAC_PRODUCT_NAME R"(.

      Usage:
      )" SERIALMACCLI_PRODUCT_NAME R"( [options]

      Options:
      -h, --help                                Show this screen.
      -v, --version                             Show version.
      -d <device>, --device=<device>            Serial port to use (e.g. "/dev/tty0", "/dev/ttyUSB0" or "COM1").
                                                If none is given, the first available port is chosen.
      -b <baudrate>, --baudrate=<baudrate>      Baud rate [default: 115200].
      -D (5-8), --data-bits=(5-8)               Data bits [default: 8].
      -P (n|o|e|s|m), --parity-bit=(n|o|e|s|m)  Parity bit mode [default: n]:
                                                n: None
                                                o: Odd
                                                e: Even
                                                s: Space
                                                m: Mark
      -S (1|2), --stop-bits=(1|2)               Stop bits [default: 1].
      -F (n|x|r|d), --flow-control=(n|x|r|d)    Flow control mode [default: n]:
                                                n: None
                                                x: XON/XOFF
                                                r: RTS/CTS
                                                d: DTR/DSR
      -C (d|r|dr|rd), --current=(d|r|dr|rd)     Current supply:
                                                d: Power DTR
                                                r: Power RTS
                                                dr or rd: Power both
      -X (i|o|io|oi), --xon-xoff=(i|o|io|oi)    XON/XOFF flow control behaviour:
                                                i: Enabled for input only
                                                o: Enabled for output only
                                                io or oi: Enabled for input and output
      -I (d|r|c|s|x), --ignore=(d|r|c|s|x)      ignore configuration options:
                                                Do not configure DTR
                                                Do not configure RTS
                                                Do not configure CTS
                                                Do not configure DSR
                                                Do not configure XON/XOFF
      -V, --verbose                             Verbosive debug information on stderr.
      )";

SerialMacCli::SerialMacCli ( )
{
}

SerialMacCli::~SerialMacCli ( )
{
  SerialMacHandler::Detach ( this );
  this->DeInitSerialPort();
  if ( mac_context )
    {
      std::free ( mac_context );
    }
}

int SerialMacCli::InitSerialPort ( std::map<std::string, docopt::value> args )
{
  sp_return sp_ret = SP_OK;
  struct sp_port **available_ports = NULL;
  docopt::value value;
  const char *port_name = NULL;
  long baudrate = 115200;
  long data_bits = 8;
  enum sp_parity parity_bit = SP_PARITY_NONE;
  long stop_bits = 1;
  enum sp_flowcontrol flow_control = SP_FLOWCONTROL_NONE;
  enum sp_rts rts = SP_RTS_FLOW_CONTROL;
  enum sp_dtr dtr = SP_DTR_FLOW_CONTROL;
  enum sp_xonxoff xon_xoff = SP_XONXOFF_DISABLED;

  value = args.at ( "--device" );
  if ( value && value.isString() )
    {
      port_name = value.asString().c_str();
      sp_ret = sp_get_port_by_name ( port_name, &port_context );
      if ( SP_OK > sp_ret || !port_context )
        {
          std::cerr << "Port \"" << port_name << "\" could not be found!"  <<
                    std::endl;
          return ( 0 == sp_ret ? 1 : sp_ret );
        }
    }
  else
    {
      /** If the user specified no port, choose any. */
      sp_ret = sp_list_ports ( &available_ports );
      if ( SP_OK > sp_ret )
        {
          return  sp_ret;
        }
      if ( available_ports[0] )
        {
          sp_ret = sp_copy_port ( available_ports[0], &port_context );
        }
      if ( available_ports )
        {
          sp_free_port_list ( available_ports );
        }
      if ( SP_OK > sp_ret  || ( !port_context ) )
        {
          std::cerr << "Could not find any serial port!" << std::endl;
          return ( 0 == sp_ret ? 1 : sp_ret );
        }
      port_name = sp_get_port_name(port_context);
    }

  sp_ret = sp_open ( port_context, SP_MODE_READ_WRITE );
  if ( SP_OK > sp_ret )
    {
      std::cerr << "Port \"" << port_name << "\" could not be opened!" <<
                std::endl;
      return sp_ret;
    }

  /** Save current port configuration for later restoring */
  sp_ret = sp_new_config ( &port_config_backup );
  if ( SP_OK > sp_ret )
    {
      std::cerr << "Config of port  \"" << port_name
                << "\" could not be saved! (Out of memory?)" << std::endl;
      return sp_ret;
    }
  sp_ret = sp_get_config ( port_context, port_config_backup );
  if ( SP_OK > sp_ret )
    {
      std::cerr << "Config of port  \"" << port_name
                << "\" could not be saved! (Read error?)" << std::endl;
      return sp_ret;
    }

  /** Create new port configuration */
  sp_ret = sp_new_config ( &port_config_new );
  if ( SP_OK > sp_ret )
    {
      std::cerr << "New config for port  \"" << port_name
                << "\" could not be set! (Out of memory?)" << std::endl;
      return sp_ret;
    }

  value = args.at ( "--baudrate" );
  if ( value )
    {
      if ( value.isString() )
        {
          baudrate = std::strtoul(value.asString().c_str(), NULL, 0);
        }
      else if ( value.isLong() )
        {
          baudrate = value.asLong();
        }
    }
  sp_ret = sp_set_config_baudrate ( port_config_new, baudrate );
  if ( SP_OK > sp_ret )
    {
      std::cerr << "Could not set baudrate to " << baudrate
                << " on port \"" << port_name << "\"!" << std::endl;
      return sp_ret;
    }

  value = args.at ( "--data-bits" );
  if ( value )
    {
      if ( value.isString() )
        {
          data_bits = std::strtoul ( value.asString().c_str(), NULL, 0 );
        }
      else if ( value.isLong() )
        {
          data_bits = value.asLong();
        }
    }
  sp_ret = sp_set_config_bits ( port_config_new, data_bits );
  if ( SP_OK > sp_ret )
    {
      std::cerr << "Could not set data bits to " << data_bits
                << " on port \"" << port_name << "\"!" << std::endl;
      return sp_ret;
    }

  value = args.at ( "--parity-bit" );
  if ( value && value.isString() )
    {
      switch ( value.asString().at ( 0 ) )
        {
        case 'n':
          parity_bit = SP_PARITY_NONE;
          break;
        case 'o':
          parity_bit = SP_PARITY_ODD;
          break;
        case 'e':
          parity_bit = SP_PARITY_EVEN;
          break;
        case 's':
          parity_bit = SP_PARITY_SPACE;
          break;
        case 'm':
          parity_bit = SP_PARITY_MARK;
          break;
        }
    }
  sp_ret = sp_set_config_parity ( port_config_new, parity_bit );
  if ( SP_OK > sp_ret )
    {
      std::cerr << "Could not set parity bit mode to " << parity_bit
                << " on port \"" << port_name << "\"!" << std::endl;
      return sp_ret;
    }

  value = args.at ( "--stop-bits" );
  if ( value )
    {
      if ( value.isString() )
        {
          stop_bits = std::strtoul ( value.asString().c_str(), NULL, 0 );
        }
      else if ( value.isLong() )
        {
          stop_bits = value.asLong();
        }
    }
  sp_ret = sp_set_config_stopbits ( port_config_new, stop_bits );
  if ( SP_OK > sp_ret )
    {
      std::cerr << "Could not set stop-bits to " << stop_bits <<
                " on port \"" << port_name << "\"!" << std::endl;
      return sp_ret;
    }

  value = args.at ( "--flow-control" );
  if ( value && value.isString() )
    {
      switch ( value.asString().at ( 0 ) )
        {
        case 'n':
          flow_control = SP_FLOWCONTROL_NONE;
          break;
        case 'x':
          flow_control = SP_FLOWCONTROL_XONXOFF;
          break;
        case 'r':
          flow_control = SP_FLOWCONTROL_RTSCTS;
          break;
        case 'd':
          flow_control = SP_FLOWCONTROL_DTRDSR;
          break;
        }
    }
  sp_ret = sp_set_config_flowcontrol ( port_config_new, flow_control );
  if ( SP_OK > sp_ret )
    {
      std::cerr << "Could not set flow-control to " << flow_control <<
                " on port \"" << port_name << "\"!" << std::endl;
      return sp_ret;
    }

  value = args.at ( "--xon-xoff" );
  if ( value && value.isString() )
    {
      if ( value.asString().length() > 1
           && ( value.asString() == "io"
                || value.asString() == "oi" )
         )
        {
          xon_xoff = SP_XONXOFF_INOUT;
        }
      switch ( value.asString().at ( 0 ) )
        {
        case 'i':
          xon_xoff = SP_XONXOFF_IN;
          break;
        case 'o':
          xon_xoff = SP_XONXOFF_OUT;
          break;
        }
      sp_set_config_xon_xoff ( port_config_new, xon_xoff );
      if ( SP_OK > sp_ret )
        {
          std::cerr << "Could not set XON/XOFF direction " <<
                    " on port \"" << port_name << "\"!" << std::endl;
          return sp_ret;
        }
    }

  value = args.at ( "--current" );
  if ( value && value.isString() )
    {
      if ( value.asString().length() > 1
           && ( value.asString() == "dr"
                || value.asString() == "rd" )
         )
        {
          rts = SP_RTS_ON;
          dtr = SP_DTR_ON;
        }
      switch ( value.asString().at ( 0 ) )
        {
        case 'd':
          dtr = SP_DTR_ON;
          break;
        case 'r':
          rts = SP_RTS_ON;
          break;
        }
      sp_set_config_rts ( port_config_new, rts );
      if ( SP_OK > sp_ret )
        {
          std::cerr << "Could not set RTS power mode " <<
                    " on port \"" << port_name << "\"!" << std::endl;
          return sp_ret;
        }
      sp_set_config_dtr ( port_config_new, dtr );
      if ( SP_OK > sp_ret )
        {
          std::cerr << "Could not set DTR power mode " <<
                    " on port \"" << port_name << "\"!" << std::endl;
          return sp_ret;
        }
    }

  value = args.at ( "--ignore" );
  if ( value && value.isString() )
    {
      switch ( value.asString().at ( 0 ) )
        {
        case 'd':
           sp_ret = sp_set_config_dtr(port_config_new, SP_DTR_INVALID);
          break;
        case 'r':
           sp_ret = sp_set_config_rts(port_config_new, SP_RTS_INVALID);
          break;
        case 'c':
           sp_ret = sp_set_config_cts(port_config_new, SP_CTS_INVALID);
          break;
        case 's':
           sp_ret = sp_set_config_dsr(port_config_new, SP_DSR_INVALID);
          break;
        case 'x':
           sp_ret = sp_set_config_xon_xoff(port_config_new, SP_XONXOFF_INVALID);
          break;
        }
      if ( SP_OK > sp_ret )
        {
          std::cerr << "Could not pin mode " <<
                    " on port \"" << port_name << "\"!" << std::endl;
          return sp_ret;
        }
    }

  sp_ret =  sp_set_config ( port_context, port_config_new );
  if ( SP_OK > sp_ret )
    {
      std::cerr << "Could not configure " <<
                " port \"" << port_name << "\"!" << std::endl;
      return sp_ret;
    }

  sp_free_config ( port_config_new );

  sp_ret = sp_new_event_set ( &port_events );
  if ( SP_OK > sp_ret )
    {
      std::cerr << "Event set for  \"" << port_name
                << "\" could not be created! (Out of memory?)" << std::endl;
      return sp_ret;
    }

  sp_ret = sp_add_port_events ( port_events,
                                port_context,
                                ( sp_event ) ( ( int ) SP_EVENT_TX_READY |
                                    ( int ) SP_EVENT_RX_READY ) );
  if ( SP_OK > sp_ret )
    {
      std::cerr << "Could not set events on port \"" << port_name << "\"!"
                << std::endl;
      return sp_ret;
    }

  port_name = sp_get_port_name ( port_context );
  if ( port_name )
    {
      std::cout << "Opened port: \"" << port_name << "\""  << std::endl;
    }

  return sp_ret;
}

void SerialMacCli::DeInitSerialPort()
{
  if ( port_events )
    {
      sp_free_event_set ( port_events );
    }
  if ( port_context )
    {
      /** Restore previous port configuration */
      sp_set_config ( port_context, port_config_backup );
      sp_free_port ( port_context );
    }
}

int SerialMacCli::Run ( int argc, char **argv )
{
  int ret = 0;
  std::map<std::string, docopt::value> args
  = docopt::docopt ( USAGE,
  { argv + 1, argv + argc },
  true,               // show help if requested
  SERIALMACCLI_VERSION_STRING, false ); // version string

  /* for debugging docopt
  for ( auto const& arg : args )
    {
      std::cout << arg.first <<  arg.second << std::endl;
    }
   */

  if ( ( ret = InitSerialPort(args) ) )
    {
      return ret;
    }

  mac_context = ( sf_serialmac_ctx* ) std::malloc ( sf_serialmac_ctx_size() );

  if ( ( ret=SerialMacHandler::Attach ( this, port_context, mac_context ) ) )
    {
      return ret;
    }
  /** Start waiting for serial input */
  std::thread halEvent ( &SerialMacCli::Wait4HalEvent, this, 100000000 );
  halEvent.detach();


  Wait4UserInput();

  return ret;
}


void SerialMacCli::Wait4UserInput ( void )
{
  std::string line = "";
  char *output_buffer = NULL;
  int output_buffer_length = 0;

  printf ( "Input text:\n" );
  getline ( std::cin, line );
  if ( line.length() > 0 )
    {
      output_buffer_length = line.length() + 1; // for the terminating '\0'
      output_buffer = ( char* ) std::malloc ( output_buffer_length );
      strncpy ( output_buffer, line.c_str(), output_buffer_length );
    }
  else
    {
      //TODO: use other means for quitting
      std::cout << "Quitting." << std::endl;
      /** Userinput was empty line -> STOP */
      run = false;
      return;
    }
  //TODO: add error handling
  sf_serialmac_tx_frame (
    mac_context, output_buffer_length,
    output_buffer,
    output_buffer_length
  );
  /** Call the callback to trigger transmission. */
  sf_serialmac_hal_tx_callback ( mac_context );
}

void SerialMacCli::Wait4HalEvent ( int nano_nap )
{
  const struct timespec nap = { /* seconds */ 0, /* nanoseconds */ nano_nap};

  while ( SP_OK <= sp_wait ( port_events, 0 ) && run )
    {
      sf_serialmac_entry ( mac_context );
      nanosleep ( &nap, NULL );
    }
  return;
}


void SerialMacCli::Update ( Event *event )
{
  if ( event->GetSource() == mac_context )
    {
      char *frame_buffer = NULL;
      size_t frame_buffer_length = event->GetDetails ( ( void** ) &frame_buffer
                                                     );
      switch ( ( SerialMacHandler::event_identifier ) event->GetIdentifier() )
        {

        case SerialMacHandler::READ_BUFFER:
          if ( frame_buffer_length )
            {
              frame_buffer = ( char* ) std::malloc ( frame_buffer_length );

              sf_serialmac_rx_frame ( ( struct sf_serialmac_ctx * ) mac_context,
                                      frame_buffer,
                                      frame_buffer_length );
            }
          break;
        case SerialMacHandler::READ_FRAME:
          if ( frame_buffer )
            {
              /** Check if a valid frame has been received */
              if ( frame_buffer_length )
                {
                  if ( '\n' == frame_buffer[0] )
                    {
                      std::cout << "Quitting." << std::endl;
                      run = false;
                    }
                  else
                    {
                      printf ( ":%s:%zd\n", frame_buffer, frame_buffer_length );
                    }
                }
              std::free ( frame_buffer );
            }
          break;
        case SerialMacHandler::WRITE_BUFFER:
          if ( frame_buffer )
            {
              std::free ( frame_buffer );
            }
          break;
        case SerialMacHandler::WRITE_FRAME:
          Wait4UserInput();
          break;
        }
    }
}
