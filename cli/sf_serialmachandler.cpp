#include "sf_serialmachandler.h"
#include "sf_serialmac.h"

//TODO: create a generic SerialMacObserver which implements Observer and is
// inherited by SerialMacCli
int SerialMacHandler::Attach ( SerialMacCli *observer,
                               struct sp_port *port_context,
                               struct sf_serialmac_ctx *mac_context )
{
  int ret = 0;

  if ( ( ret = sf_serialmac_init ( mac_context,
                                   ( void * ) port_context,
                                   ( SF_SERIALMAC_HAL_READ_FUNCTION )
                                   sp_nonblocking_read,
                                   ( SF_SERIALMAC_HAL_READ_WAIT_FUNCTION )
                                   sp_input_waiting,
                                   ( SF_SERIALMAC_HAL_WRITE_FUNCTION )
                                   sp_nonblocking_write,
                                   ( SF_SERIALMAC_EVENT ) ReadFrameEvent,
                                   ( SF_SERIALMAC_EVENT ) ReadBufferEvent,
                                   ( SF_SERIALMAC_EVENT ) WriteFrameEvent,
                                   ( SF_SERIALMAC_EVENT ) WriteBufferEvent ) ) )
    {
      return ret;
    }

  Subject::Attach ( observer );

  return ret;
}

bool SerialMacHandler::filter ( Observer *observer, Event *event )
{
  /* This test is merely done to avoid warnings about unused parameters */
  if ( observer && event )
    {
      //   return ( ( ( SerialMacCli* ) observer )->GetSerialMacContext() ==
      //            event->GetSource() ) ? true : false;
      /* Real filtering is left to the observers */
      return true;
    }
  else
    {
      return false;
    }
}

void SerialMacHandler::ReadFrameEvent ( void *mac_context,
                                        char *frame_buffer,
                                        size_t frame_buffer_length )
{
  Event event ( READ_FRAME, mac_context, ( void* ) frame_buffer,
                frame_buffer_length );
  Subject::Notify ( &event, ( Filter ) filter );
}

void SerialMacHandler::ReadBufferEvent ( void *mac_context,
    char *nullpointer,
    size_t frame_buffer_length )
{
  Event event ( READ_BUFFER, mac_context, ( void* ) nullpointer,
                frame_buffer_length
              );
  Subject::Notify ( &event, ( Filter ) filter );
}

/**
 * Function to be called by the MAC when a whole frame has been sent.
 */
void SerialMacHandler::WriteFrameEvent ( void *mac_context, char *nullpointer,
    size_t frame_length )
{
  Event event ( WRITE_FRAME, mac_context, nullpointer, frame_length );
  Subject::Notify ( &event, ( Filter ) filter );
}

/**
 * Function to be called by the MAC when an outgoing buffer has been processed.
 */
void SerialMacHandler::WriteBufferEvent ( void *mac_context, char *frame_buffer,
    size_t frame_buffer_length )
{
  Event event ( WRITE_BUFFER, mac_context, frame_buffer, frame_buffer_length );
  Subject::Notify ( &event, ( Filter ) filter );
}
