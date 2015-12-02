#include <iostream>
#include <thread>
#include <string.h>
#include <algorithm>
#include <docopt.h>

#include "sf_serialmaccli.h"
#include "sf_serialmachandler.h"
#include "sf_serialmac.h"
#include "version.h"


    static const char USAGE[] =
      SERIALMAC_PRODUCT_NAME R"(.

      Usage:
      )" SERIALMACCLI_PRODUCT_NAME R"( [options] [<payload> ...]

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

SerialMacCli::SerialMacCli ( int argc, char **argv )
{
  docopt::value value;
  args = docopt::docopt ( USAGE,
  { argv + 1, argv + argc },
  true,               // show help if requested
  SERIALMACCLI_VERSION_STRING, false ); // version string

  value = args.at ( "--verbose" );
  if ( value && value.isBool() )
    {
      if ( value.asBool() )
        {
          Verbose = std::printf;
        }
      else
        {
          Verbose = NonVerbose;
        }
    }

  /* for debugging docopt
  for ( auto const& arg : args )
    {
      std::cout << arg.first << ": " <<  arg.second << std::endl;
    }
   */

  mac_context = ( sf_serialmac_ctx* ) std::malloc ( sf_serialmac_ctx_size() );
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

/**
 * This is a dummy function which is used instead of printf in non-verbose
 * mode.
 */
int SerialMacCli::NonVerbose (const char *format, ...){
  return strlen(format);
}

/**
 * Initialize the serial port.
 */
int SerialMacCli::InitSerialPort ( )
{
  sp_return sp_ret = SP_OK;
  struct sp_port **available_ports = NULL;
  docopt::value value;
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

  sp_ret = sp_new_event_set ( &port_rx_event );
  if ( SP_OK > sp_ret )
    {
      std::cerr << "RX event set for  \"" << port_name
                << "\" could not be created! (Out of memory?)" << std::endl;
      return sp_ret;
    }

  sp_ret = sp_add_port_events ( port_rx_event,
                                port_context,
                                SP_EVENT_RX_READY );
  if ( SP_OK > sp_ret )
    {
      std::cerr << "Could not set RX event on port \"" << port_name << "\"!"
                << std::endl;
      return sp_ret;
    }

  sp_ret = sp_new_event_set ( &port_tx_event );
  if ( SP_OK > sp_ret )
    {
      std::cerr << "TX event set for  \"" << port_name
                << "\" could not be created! (Out of memory?)" << std::endl;
      return sp_ret;
    }

  sp_ret = sp_add_port_events ( port_tx_event,
                                port_context,
                                SP_EVENT_TX_READY );
  if ( SP_OK > sp_ret )
    {
      std::cerr << "Could not set TX event on port \"" << port_name << "\"!"
                << std::endl;
      return sp_ret;
    }

  port_name = sp_get_port_name ( port_context );
  if ( port_name )
    {
      Verbose( "Opened port: \"%s\"\n",  port_name );
    }

  return sp_ret;
}

void SerialMacCli::DeInitSerialPort()
{
  if ( port_rx_event )
    {
      sp_free_event_set ( port_rx_event );
    }
  if ( port_tx_event )
    {
      sp_free_event_set ( port_tx_event );
    }
  if ( port_context )
    {
      /** Restore previous port configuration */
      sp_set_config ( port_context, port_config_backup );
      sp_free_port ( port_context );
    }
}


/**
 * Using a template allows us to ignore the differences between functors,
 * function pointers and lambda
 */
template<typename IfFunc, typename ElseFunc>
/**
 * To avoid code rendundancy this function executes the IfOperation if
 * payload has been passed as parameter and the ElseOperation otherwise.
 */
void SerialMacCli::PayloadPassedAsParameter (
  IfFunc IfOperation, ElseFunc ElseOperation )
{
  docopt::value value = args.at ( "<payload>" );

  if ( value && value.isStringList()
       && value.asStringList().size() != 0 )
    {
      return IfOperation (value);
    }
  return ElseOperation();
}


void SerialMacCli::CliInput ( void )
{
  bool run = true;
  std::string line = "";
  char *output_buffer = NULL;
  int output_buffer_length = 0;

  /** Repeat until the user stops you */
  while ( run )
    {
      switch ( cli_input_state )
        {
        case CLI:
          PayloadPassedAsParameter ( [&line, this] ( docopt::value value )
          {
            std::vector <std::string> line_as_list = value.asStringList();
            std::for_each ( line_as_list.begin(), line_as_list.end(),
                            /**
                             * A lambda within a lambda! But this is how
                             * for_each works
                             */
                            [&line] ( std::string& word )
            {
              return line+=word;
            } );
          }, [&line, this] ()
          {
            Verbose ( "Input text:\n" );
            getline ( std::cin, line );
          } );
          if ( line.length() > 0 )
            {
              /**
               * On Windows the pointer given by line.c_str() cannot be passed
               * to sf_serialmac_tx_frame() because the memory is invalidated.
               * Therefore the zero terminated character array is copied to
               * output_buffer first.
               */
              output_buffer_length = line.length() + 1/* for the terminating '\0' */;
              /** Will be freed in Update() when TX has been completed. */
              output_buffer = ( char* ) std::malloc ( output_buffer_length );
              strncpy ( output_buffer, line.c_str(), output_buffer_length );
              //TODO: add error handling
              /** Trigger TX */
              sf_serialmac_tx_frame (
                mac_context, output_buffer_length,
                output_buffer,
                output_buffer_length
              );
              cli_input_state = SERIAL;
            }
          else
            {
              cli_input_state = QUIT;
            }
          break;

        case SERIAL:
          /**
           * Call the callback to process transmission until Update() sets
           * cli_input_state to CLI.
           */
          sf_serialmac_hal_tx_callback ( mac_context );
          if ( SP_OK != sp_wait ( port_tx_event, 0 ) )
            {
              std::cerr << "Error during transmission on \"" << port_name
              << "\"!"<< std::endl;
            }
          break;

        case QUIT:
          //TODO: use other means for quitting
          Verbose ( "Quitting.\n" );
          /** Userinput was empty line -> STOP */
          run = false;
          /** This stops the other thread. */
          cli_output_state = QUIT;
          break;
        }
    }
}

void SerialMacCli::CliOutput ( void )
{
  bool run = true;

  while ( run )
    {
      switch ( cli_output_state )
        {
        case SERIAL:
          /**
           * Start waiting for serial input.
           * A timeout is given to quit on demand. */
          if ( SP_OK != sp_wait ( port_rx_event, 100 ) )
            {
              std::cerr << "Error during reception on \"" << port_name
                        << "\"!"<< std::endl;
            }
          sf_serialmac_hal_rx_callback ( mac_context );
          break;
        case CLI:
          // currently not used
          break;
        case QUIT:
          Verbose( "Quitting.\n" );
          /** This stops the other thread. */
          cli_input_state = QUIT;
          run = false;
          break;
        }
    }
}


int SerialMacCli::Run ( )
{
  int ret = InitSerialPort ( );

  if(ret){
    std::cerr << "Could not initialize \"" << port_name
                        << "\"!"<< std::endl;
    return ret;
  }

  ret = SerialMacHandler::Attach ( this, port_context, mac_context );
  if(ret){
    std::cerr << "Could not initialize serial MAC on \"" << port_name
                        << "\"!"<< std::endl;
    return ret;
  }

  /** Start waiting for CLI input */
  cli_input_state = CLI;
  std::thread process_cli_input (&SerialMacCli::CliInput, this);

  /** Start waiting for CLI output */
  cli_output_state = SERIAL;
  CliOutput();

  /**
   * 30.3.1.3 thread destructor [thread.thread.destr]
   *
   * ~thread();
   *
   * If joinable() then terminate(), otherwise no effects.
   * [ Note: Either implicitly detaching or joining a joinable()
   * thread in its destructor could result in difficult to debug
   * correctness (for detach) or performance (for join) bugs
   * encountered only when an exception is raised. Thus the
   * programmer must ensure that the destructor is never executed
   * while the thread is still joinable. — end note ]
   */
  process_cli_input.join();

  return ret;
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
              /**
               * For future implementations of cli_output_state = CLI:
               * Switch back to serial state.
               */
              cli_output_state = SERIAL;
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
                      cli_output_state = QUIT;
                    }
                  else
                    {
                      /**
                       * Instead of printing directly, cli_output_state = CLI
                       * could be set and frame_buffer could be processed in
                       * CliOutput.
                       */
                      printf ( "%s\n", frame_buffer );
                      Verbose ( "Length:\n%zd\n", frame_buffer_length );
                    }
                }
              /**
               * TODO: if (in future implementations) frame_buffer is not
               * processed directly here do not free it!
               */
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
          PayloadPassedAsParameter (
            [this] ( docopt::value value )
          {
            /**
             * Silence the warning: "unused parameter" because we really do not
             * need it here.
             */
            ( void ) value;
            /** Payload passed as parameter has been send. */
            cli_input_state = QUIT;
          }, [this] ()
          {
            cli_input_state = CLI;
          } );
          break;
        }
    }
}
