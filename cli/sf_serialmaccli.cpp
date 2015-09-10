#include <iostream>
#include <thread>
#include <unistd.h>
#include <string.h>

#include "sf_observer.h"
#include "sf_serialmachandler.h"
#include "version.h"
#include "sf_serialmaccli.h"

#define SF_SERIAL_INPUT_MAX_SIZE 255

SerialMacCli::SerialMacCli ( const char* portname ) //: port_name()TODO why does this not work?
{
  port_name = portname;
  input_buffer = ( char* ) std::malloc ( SF_SERIAL_INPUT_MAX_SIZE );
  output_buffer = ( char* ) std::malloc ( SF_SERIAL_INPUT_MAX_SIZE );
  SerialMacHandler::Attach ( this );
}

SerialMacCli::~SerialMacCli ( )
{
  free ( mac_context );
  free ( input_buffer );
  free ( output_buffer );
}

void* SerialMacCli::CreateSerialMacContext ( size_t size )
{
  if ( NULL==mac_context )
    {
      /** The serial MAC does not do any memory management */
      mac_context = std::malloc ( size );
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

void SerialMacCli::Run()
{
  /** Start waiting for user input */
  std::thread userInputEvent ( &SerialMacCli::Wait4UserInput, this );
  userInputEvent.detach();
}


void SerialMacCli::Wait4UserInput ( void )
{
  enum sf_serialmac_return ret = SF_SERIALMAC_SUCCESS;
  std::string line = "";
  const size_t frmLength = 9;

  if ( !oBuffRemains )
    {
      printf ( "Input text:\n" );
      getline ( std::cin, line );
      if ( line.length() > 0 )
        {
          oBuffLength = oBuffRemains = line.length();
          strncpy ( output_buffer, line.c_str(), sizeof output_buffer );//FIXME
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
      while ( ( ret = sf_serialmac_tx_frame ( mac_ctx, frmLength,
                                              output_buffer + ( oBuffLength - oBuffRemains ),
                                              oBuffRemains ) ) != SF_SERIALMAC_SUCCESS )
        {
          std::cerr << "TX Error " << ret << "\nline: "
                    << output_buffer + ( oBuffLength - oBuffRemains )
                    << "\nlength: "
                    << oBuffRemains
                    << std::endl;
          sleep ( 1 );
        }
      //        switch (status)
//        {
//        case START_FRAME:
//            if((ret = sf_serialmac_txFrameStart(mac_ctx,
//                                                 frmLength)) != SF_SERIALMAC_SUCCESS)
//            {
//                printf("Frame Error %i\n", ret);
//            }
//            status = APPEND_FRAME;
//        //break; omitted
//        case APPEND_FRAME:
//            while((ret = sf_serialmac_txFrameAppend(mac_ctx,
//                         oBuff + (oBuffLength - oBuffRemains),
//                         oBuffRemains)) != SF_SERIALMAC_SUCCESS)
//            {
//                printf("TX Error %i\nline: %s\nlength: %zd\n", ret,
//                       oBuff + (oBuffLength - oBuffRemains), oBuffRemains);
//                sleep(1);
//            }
//            break;
//        default:
//            printf("Exception Error\n");
//            break;
//        }
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
      break;
    }
}
