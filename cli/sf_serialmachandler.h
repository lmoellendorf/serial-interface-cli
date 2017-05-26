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

#ifndef SERIALMACHANDLER_H
#define SERIALMACHANDLER_H

#include "sf_subject.h"
#include "sf_serialmaccli.h"

namespace sf
{

class SerialMacHandler : public Subject
{
public:
    enum event_identifier {
        READ_BUFFER,
        READ_FRAME,
        WRITE_FRAME,
        WRITE_BUFFER,
        SYNC_BYTE,
    };
    static int Attach ( SerialMacCli *serialmaccli,
                        struct sp_port *port_context,
                        struct sf_serialmac_ctx *mac_context
                      );

private:
    static void ReadFrameEvent ( void *mac_context,
                            char *frame_buffer,
                            size_t frame_buffer_length
                          );
    static void ReadBufferEvent ( void *mac_context,
                                char *frame_buffer,
                                size_t frame_buffer_length
                              );
    static void WriteFrameEvent ( void *mac_context,
                             char *nullpointer,
                             size_t frame_length
                           );
    static void WriteBufferEvent ( void *mac_context,
                                char *frame_buffer,
                                size_t frame_buffer_length
                              );
    static void SyncByteEvent ( void *mac_context,
                                char *nullpointer,
                                size_t frame_buffer_length
                            );
    static bool filter ( Observer *observer, Event *event );

};

}
#endif // SERIALMACHANDLER_H
