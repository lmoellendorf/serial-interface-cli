#include <string>
#include <time.h>
#include <thread>
#include <libserialport.h>

#include "sf_subject.h"
#include "sf_serialmac.h"
#include "sf_serialmaccli.h"

#include "sf_serialmachandler.h"

//TODO: make all defines obsolete:
#define SF_SERIAL_BAUDRATE 115200
#define SF_SERIAL_BITS 8
#define SF_SERIAL_STOPBITS 1
#define SF_SERIAL_FLOWCTRL SP_FLOWCONTROL_NONE

int SerialMacHandler::Attach ( SerialMacCli* serialmaccli )
{
  int ret = 0;
  /** Here the memory is allocated by serialmaccli */
  struct sf_serialmac_ctx *mac_ctx = ( struct sf_serialmac_ctx* ) serialmaccli->CreateSerialMacContext ( sf_serialmac_ctx_size() );
  const char *portname = serialmaccli->GetSerialPortName();

  /** Here we need a pointer to pointer because the memory will be allocated by libserialport */
  struct sp_port **port = ( struct sp_port** ) serialmaccli->GetSerialPortContext();
  struct sp_port_config **saved_port_config = ( struct sp_port_config** ) serialmaccli->GetSerialPortConfig();
  struct sp_event_set **port_events = ( struct sp_event_set** ) serialmaccli->GetSerialPortRxEvents();

  if ( ( ret =  InitSerialPort ( port, portname, saved_port_config, port_events ) ) )
    {
      return ret;
    }
  if ( ( ret = sf_serialmac_init ( mac_ctx,
                                   ( void * ) *port,
                                   ( SF_SERIALMAC_HAL_READ_FUNCTION ) sp_nonblocking_read,
                                   ( SF_SERIALMAC_HAL_READ_WAIT_FUNCTION ) sp_input_waiting,
                                   ( SF_SERIALMAC_HAL_WRITE_FUNCTION ) sp_nonblocking_write,
                                   ReadEvent, BufferRxEvent,
                                   WriteEvent, BufferTxEvent ) ) )
    {
      return ret;
    }

  /** Start waiting for serial input */
  std::thread halRxEvent ( &SerialMacHandler::Wait4HalEvent, *port, *port_events, SP_EVENT_RX_READY, mac_ctx, sf_serialmac_hal_rx_callback );
  halRxEvent.detach();

  Subject::Attach ( serialmaccli );
  return ret;
}

void SerialMacHandler::Detach ( SerialMacCli* serialmaccli )
{
  struct sp_port **port = ( struct sp_port** ) serialmaccli->GetSerialPortContext();
  struct sp_port_config **saved_port_config = ( struct sp_port_config** ) serialmaccli->GetSerialPortConfig();
  struct sp_event_set **port_events = ( struct sp_event_set** ) serialmaccli->GetSerialPortRxEvents();

  if ( NULL !=  port && NULL != *port )
    {
      /** Restore previous port configuration */
      sp_set_config ( *port, *saved_port_config );
      sp_free_event_set ( *port_events );
      sp_free_port ( *port );
    }

  Subject::Detach ( serialmaccli );
}

bool SerialMacHandler::filter ( Observer *observer, Event *event )
{
  return ( ( ( SerialMacCli* ) observer )->GetSerialMacContext() == event->GetSource() ) ? true : false;
}

void SerialMacHandler::ReadEvent ( void *mac_context, char *frame_buffer, size_t frame_buffer_length )
{
  std::forward_list<Observer*> filtered_observers;
  Event event ( READ, mac_context, ( void* ) frame_buffer, frame_buffer_length );

  if ( frame_buffer && frame_buffer_length )
    {
      //TODO: lambda function?
      Subject::Notify ( &event, ( Filter ) filter );
    }
}

void SerialMacHandler::BufferRxEvent ( void *mac_context,
                                       char *frame_buffer,
                                       size_t frame_buffer_length )
{
  if ( NULL != frame_buffer )
    {
      free ( frame_buffer );
    }
  else
    {
      char *new_frame_buffer = ( char* ) std::malloc ( frame_buffer_length );

      sf_serialmac_rx_frame ( ( struct sf_serialmac_ctx * ) mac_context,
                              new_frame_buffer,
                              frame_buffer_length );
    }
}

void SerialMacHandler::WriteEvent ( void *mac_context, size_t processed ) //FIXME
{

  //status = START_FRAME;
}

void SerialMacHandler::BufferTxEvent ( void *mac_context, size_t processed ) //FIXME
{

  //oBuffRemains -= processed;
  //thread userInputEventLoop ( Wait4UserInput, this );
  //userInputEventLoop.detach();
}

int SerialMacHandler::InitSerialPort ( sp_port **port, const char *portname, sp_port_config **saved_port_config, sp_event_set **port_events )
{
  sp_return sp_ret = SP_OK;
  struct sp_port **available_ports = NULL;

  /** If the user specified no port, choose any. */
  if ( NULL == portname )
    {
      sp_ret = sp_list_ports ( &available_ports );
      if ( SP_OK > sp_ret )
        {
          return  sp_ret;
        }
      if ( NULL != available_ports[0] )
        {
          sp_ret = sp_copy_port ( available_ports[0], port );
        }
      if ( NULL != available_ports )
        {
          sp_free_port_list ( available_ports );
        }
      if ( SP_OK > sp_ret  || ( NULL !=  port && NULL == *port ) )
        {
//           cerr << "Could not find any serial port!\n" << endl;
          return ( 0 == sp_ret ? 1 : sp_ret );
        }
    }
  else
    {
      sp_ret = sp_get_port_by_name ( portname, port );
      if ( SP_OK > sp_ret || ( NULL !=  port && NULL == *port ) )
        {
//           cerr << "Port \"" << portname << "\" could not be found!\n" << endl;
          return ( 0 == sp_ret ? 1 : sp_ret );
        }
    }

  sp_ret = sp_open ( *port, SP_MODE_READ_WRITE );
  if ( SP_OK > sp_ret )
    {
//       cerr << "Port \"" << portname << "\" could not be opened!\n" << endl;
      return sp_ret;
    }

  /** Save current port configuration for later restoring */
  sp_ret = sp_new_config ( saved_port_config );
  if ( SP_OK > sp_ret )
    {
//       cerr << "Config of port  \"" << portname << "\" could not be saved! (Out of memory?)\n" << endl;
      return sp_ret;
    }
  sp_ret = sp_get_config ( *port, *saved_port_config );
  if ( SP_OK > sp_ret )
    {
//       cerr << "Config of port  \"" << portname << "\" could not be saved! (Read error?)\n" << endl;
      return sp_ret;
    }

  sp_ret = sp_set_baudrate ( *port, SF_SERIAL_BAUDRATE );
  if ( SP_OK > sp_ret )
    {
//       cerr << "Could not set baudrate to " << SF_SERIAL_BAUDRATE << " on port \"" << portname << "\"!\n" << endl;
      return sp_ret;
    }

  sp_ret = sp_set_bits ( *port, SF_SERIAL_BITS );
  if ( SP_OK > sp_ret )
    {
//       cerr << "Could not set number of bits to " << SF_SERIAL_BITS << " on port \"" << portname << "\"!\n" << endl;
      return sp_ret;
    }

  sp_ret = sp_set_parity ( *port, SP_PARITY_NONE );
  if ( SP_OK > sp_ret )
    {
//       cerr << "Could not set parity to " << SP_PARITY_NONE << " on port \"" << portname << "\"!\n" << endl;
      return sp_ret;
    }

  sp_ret = sp_set_stopbits ( *port, SF_SERIAL_STOPBITS );
  if ( SP_OK > sp_ret )
    {
//       cerr << "Could not set stop-bits to " << SF_SERIAL_STOPBITS << " on port \"" << portname << "\"!\n" << endl;
      return sp_ret;
    }

  sp_ret = sp_set_flowcontrol ( *port, SF_SERIAL_FLOWCTRL );
  if ( SP_OK > sp_ret )
    {
//       cerr << "Could not set flow-control to " << SF_SERIAL_FLOWCTRL << " on port \"" << portname << "\"!\n" << endl;
      return sp_ret;
    }

  sp_ret = sp_new_event_set ( port_events );
  if ( SP_OK > sp_ret )
    {
      return sp_ret;
    }

  return 0;
}

void SerialMacHandler::Wait4HalEvent ( sp_port *port,
                                       sp_event_set *port_events,
                                       enum sp_event event,
                                       struct sf_serialmac_ctx *mac_ctx,
                                       enum sf_serialmac_return ( *sf_serialmac_hal_callback ) ( struct sf_serialmac_ctx *ctx ) )
{
//   struct timespec
//   {
//     time_t tv_sec;        /* seconds */
//     long   tv_nsec;       /* nanoseconds */
//   };

  const struct timespec nap = { 0, 100000000};

  if ( SP_OK <= sp_add_port_events ( port_events, port, event ) )
    {
      while ( SP_OK <= sp_wait ( port_events, 0 ) )
        {
          sf_serialmac_hal_callback ( mac_ctx );
          nanosleep ( &nap, NULL ); // TODO really needed?
        }
    }
  return;
}

/* TODO This would be an alternative */
//  thread runSerialMacLoop ( &SerialMacCli::RunSerialMac, this );
//  runSerialMacLoop.detach();

// void SerialMacCli::RunSerialMac ( ) //T
// {
//   /* Loop until the user quits */
//   while ( run )
//     {
//       sf_serialmac_entry ( ( struct sf_serialmac_ctx * ) mac_ctx );
//     }
// }
