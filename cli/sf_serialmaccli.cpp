#include <iostream>
#include <thread>
#include <string.h>

#include "sf_serialmaccli.h"
#include "sf_serialmachandler.h"
#include "sf_serialmac.h"


#define SF_SERIAL_INPUT_MAX_SIZE 255
//TODO: make all defines obsolete:
#define SF_SERIAL_BAUDRATE 115200
#define SF_SERIAL_BITS 8
#define SF_SERIAL_STOPBITS 1
#define SF_SERIAL_FLOWCTRL SP_FLOWCONTROL_NONE

SerialMacCli::SerialMacCli ( const char* port_name )
//: port_name()TODO why does this not work?
{
  this->port_name = port_name;
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

int SerialMacCli::InitSerialPort ()
{
  sp_return sp_ret = SP_OK;
  struct sp_port **available_ports = NULL;

  /** If the user specified no port, choose any. */
  if ( !port_name )
    {
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
    }
  else
    {
      sp_ret = sp_get_port_by_name ( port_name, &port_context );
      if ( SP_OK > sp_ret || !port_context  )
        {
          std::cerr << "Port \"" << port_name << "\" could not be found!"  <<
                    std::endl;
          return ( 0 == sp_ret ? 1 : sp_ret );
        }
    }

  sp_ret = sp_open ( port_context, SP_MODE_READ_WRITE );
  if ( SP_OK > sp_ret )
    {
      std::cerr << "Port \"" << port_name << "\" could not be opened!" <<
                std::endl;
      return sp_ret;
    }

  /** Save current port configuration for later restoring */
  sp_ret = sp_new_config ( &saved_port_config );
  if ( SP_OK > sp_ret )
    {
      std::cerr << "Config of port  \"" << port_name
                << "\" could not be saved! (Out of memory?)" << std::endl;
      return sp_ret;
    }
  sp_ret = sp_get_config ( port_context, saved_port_config );
  if ( SP_OK > sp_ret )
    {
      std::cerr << "Config of port  \"" << port_name
                << "\" could not be saved! (Read error?)" << std::endl;
      return sp_ret;
    }

  sp_ret = sp_set_baudrate ( port_context, SF_SERIAL_BAUDRATE );
  if ( SP_OK > sp_ret )
    {
      std::cerr << "Could not set baudrate to " << SF_SERIAL_BAUDRATE
                << " on port \"" << port_name << "\"!" << std::endl;
      return sp_ret;
    }

  sp_ret = sp_set_bits ( port_context, SF_SERIAL_BITS );
  if ( SP_OK > sp_ret )
    {
      std::cerr << "Could not set number of bits to " << SF_SERIAL_BITS
                << " on port \"" << port_name << "\"!" << std::endl;
      return sp_ret;
    }

  sp_ret = sp_set_parity ( port_context, SP_PARITY_NONE );
  if ( SP_OK > sp_ret )
    {
      std::cerr << "Could not set parity to " << SP_PARITY_NONE
                << " on port \"" << port_name << "\"!" << std::endl;
      return sp_ret;
    }

  sp_ret = sp_set_stopbits ( port_context, SF_SERIAL_STOPBITS );
  if ( SP_OK > sp_ret )
    {
      std::cerr << "Could not set stop-bits to " << SF_SERIAL_STOPBITS <<
                " on port \"" << port_name << "\"!" << std::endl;
      return sp_ret;
    }

  sp_ret = sp_set_flowcontrol ( port_context, SF_SERIAL_FLOWCTRL );
  if ( SP_OK > sp_ret )
    {
      std::cerr << "Could not set flow-control to " << SF_SERIAL_FLOWCTRL <<
                " on port \"" << port_name << "\"!" << std::endl;
      return sp_ret;
    }

  sp_ret = sp_new_event_set ( &port_events );
  if ( SP_OK > sp_ret )
    {
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

  port_name = sp_get_port_name(port_context);
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
      sp_set_config ( port_context, saved_port_config );
      sp_free_port ( port_context );
    }
}

int SerialMacCli::Run()
{
  int ret = 0;

  if ( ( ret = InitSerialPort() ) )
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
