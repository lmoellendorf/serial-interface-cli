#ifndef SERIALMACCLI_H
#define SERIALMACCLI_H

#include <string>
extern "C"
{
#include <libserialport.h>
}
#include "sf_serialmac.h"
#include "sf_observer.h"
class Observer;

class SerialMacCli: public Observer
{
public:
    SerialMacCli (const char* portname);
    ~SerialMacCli ( );

    //TODO: API
    void Update ( Event *event );
    void* CreateSerialMacContext ( size_t size );
    void* GetSerialMacContext ( );
    const char* GetSerialPortName();
    void** GetSerialPortContext();
    void** GetSerialPortConfig();
    void** GetSerialPortRxEvents();

    void Run ( void );

private:
    void *port_config;
    const char *port_name;
    void *port_context;
    void *port_rx_events;
    void *mac_context = NULL;

    enum state {
        START_FRAME,
        APPEND_FRAME,
    };
    int run = true;
    int status = START_FRAME;
    //struct sp_port *port = NULL;
    struct sf_serialmac_ctx *mac_ctx;
    size_t iBuffLen = 0;
    size_t oBuffRemains = 0;
    size_t oBuffLength = 0;
    char *input_buffer;
    char *output_buffer;
    void RunSerialMac ( void );
    void Wait4UserInput ( void );
    void Wait4HalTxEvent ( void );
    void Wait4HalRxEvent ( void );
    void Wait4HalEvent ( enum sp_event event,
                         enum sf_serialmac_return ( *sf_serialmac_halCb ) ( struct sf_serialmac_ctx *ctx ) );
};

#endif // SERIALMACCLI_H
