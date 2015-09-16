#include <iostream>
#include <thread>
#include <unistd.h>
#include <string.h>

#include "sf_observer.h"
#include "sf_serialmachandler.h"
#include "version.h"
#include "sf_serialmaccli.h"

#define SF_SERIAL_INPUT_MAX_SIZE 255

SerialMacCli::SerialMacCli ( const char* portname )
//: port_name()TODO why does this not work?
{
  port_name = portname;
  input_buffer = ( char* ) std::malloc ( SF_SERIAL_INPUT_MAX_SIZE );
  output_buffer = ( char* ) std::malloc ( SF_SERIAL_INPUT_MAX_SIZE );
}

SerialMacCli::~SerialMacCli ( )
{
  SerialMacHandler::Detach ( this );
  free ( mac_context );
  free ( input_buffer );
  free ( output_buffer );
}

void* SerialMacCli::GetSerialMacContext ( size_t size )
{
  if ( NULL==mac_context )
    {
      /** The serial MAC does not do any memory management */
      mac_context = ( struct sf_serialmac_ctx* ) std::malloc ( size );
    }
  return mac_context;
}

void* SerialMacCli::GetSerialMacContext()
{
  return mac_context;
}


void** SerialMacCli::GetSerialPortContext()
{
  return &port_context;
}

void** SerialMacCli::GetSerialPortConfig()
{
  return &port_config;
}

void** SerialMacCli::GetSerialPortRxEvents()
{
  return &port_rx_events;
}

const char* SerialMacCli::GetSerialPortName()
{
  return port_name;
}

int SerialMacCli::Run()
{
  int ret = 0;
  if ( ( ret=SerialMacHandler::Attach ( this ) ) )
    {
      return ret;
    }
  /** Start waiting for user input */
  std::thread userInputEvent ( &SerialMacCli::Wait4UserInput, this );
  userInputEvent.detach();
  return ret;
}


void SerialMacCli::Wait4UserInput ( void )
{
  std::string line = "";

  if ( !oBuffRemains )
    {
      printf ( "Input text:\n" );
      getline ( std::cin, line );
      if ( line.length() > 0 )
        {
          oBuffLength = oBuffRemains = line.length();
          strncpy ( output_buffer, line.c_str(), SF_SERIAL_INPUT_MAX_SIZE );
        }
      else
        {
          /** Userinput was empty line -> STOP */
          run = false;
          printf ( "Stop\n" );
        }
    }
  if ( oBuffRemains )
    {
      SerialMacHandler::Tx ( this, output_buffer, oBuffRemains );
    }
}

void SerialMacCli::Update ( Event *event )
{
  switch ( ( SerialMacHandler::event_identifier ) event->GetIdentifier() )
    {
      char *frame_buffer;
      size_t frame_buffer_length;

    case SerialMacHandler::READ:
      frame_buffer_length = event->GetDetails ( ( void** ) &frame_buffer );
      if ( frame_buffer && frame_buffer_length )
        {

          if ( '\n' == frame_buffer[0] )
            {
              run = false;
            }
          else
            {
              printf ( ":%s:%zd\n", frame_buffer, frame_buffer_length );
            }
        }
      break;
    case SerialMacHandler::WRITE:
      void *dummy = NULL;//FIXME
      oBuffRemains -= event->GetDetails ( &dummy );
      Wait4UserInput();
      break;
    }
}
