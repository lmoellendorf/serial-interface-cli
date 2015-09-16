#ifndef SERIALMACHANDLER_H
#define SERIALMACHANDLER_H

#include "sf_subject.h"
#include "sf_serialmaccli.h"

class SerialMacHandler : public Subject
{
public:
    enum event_identifier {
        READ,
        WRITE,
    };
    static int Attach ( SerialMacCli* serialmaccli );
    static void Detach ( SerialMacCli* serialmaccli );
    static void Notify ( Event *event );
    static void Tx ( SerialMacCli* serialmaccli, char *buffer, size_t length );

private:
    static int InitSerialPort ( sp_port **port, const char *portname, sp_port_config **saved_port_config, sp_event_set **port_events );
    static void Wait4HalEvent ( sp_port *port,
                                sp_event_set *port_events,
                                enum sp_event event,
                                struct sf_serialmac_ctx
                                *mac_context,
                                enum sf_serialmac_return
                                ( *sf_serialmac_hal_callback )
                                ( struct sf_serialmac_ctx* ctx ),
                                int nanonap
                              );
    static void ReadEvent ( void *mac_context, char *frame_buffer, size_t frame_buffer_length );
    static void BufferRxEvent ( void *mac_context, char *frame_buffer, size_t frame_buffer_length );
    static void WriteEvent ( void *mac_context, size_t processed );
    static void BufferTxEvent ( void *mac_context, size_t processed );
    static bool filter ( Observer *observer, Event *event );

};

#endif // SERIALMACHANDLER_H
