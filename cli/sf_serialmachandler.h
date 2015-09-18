#ifndef SERIALMACHANDLER_H
#define SERIALMACHANDLER_H

#include "sf_subject.h"
#include "sf_serialmaccli.h"

class SerialMacHandler : public Subject
{
public:
    enum event_identifier {
        READ_BUFFER,
        READ_FRAME,
        WRITE_FRAME,
        WRITE_BUFFER,
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
    static bool filter ( Observer *observer, Event *event );

};

#endif // SERIALMACHANDLER_H
