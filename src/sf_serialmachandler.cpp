/**
 * @code
 *  ___ _____ _   ___ _  _____ ___  ___  ___ ___
 * / __|_   _/_\ / __| |/ / __/ _ \| _ \/ __| __|
 * \__ \ | |/ _ \ (__| ' <| _| (_) |   / (__| _|
 * |___/ |_/_/ \_\___|_|\_\_| \___/|_|_\\___|___|
 * embedded.connectivity.solutions.==============
 * @endcode
 *
 * @file
 * @copyright  STACKFORCE GmbH, Heitersheim, Germany, http://www.stackforce.de
 * @author     STACKFORCE
 * @author     Lars Möllendorf
 * @brief      STACKFORCE Serial MAC Command Line Client
 *
 * @details See @code sf --help @endcode for details.
 *
 * This file is part of the STACKFORCE Serial Command Line Client
 * (below "serialmac cli").
 *
 * The serialmac cli is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The serialmac cli is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with libserialmac.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "sf_serialmachandler.h"
#include "sf_serialmac.h"

namespace sf
{

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
                                   ( SF_SERIALMAC_EVENT ) SyncByteEvent,
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

/**
 * Function to be called by the MAC when a sync byte has been processed.
 */
void SerialMacHandler::SyncByteEvent(void* mac_context, char* nullpointer, size_t frame_buffer_length)
{
    Event event(SYNC_BYTE, mac_context, nullpointer, frame_buffer_length);
    Subject::Notify( &event, ( Filter) filter );
}


}
