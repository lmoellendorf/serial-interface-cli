#ifndef SERIALMACCLI_H
#define SERIALMACCLI_H

extern "C"
{
#include <libserialport.h>
}
#include "sf_observer.h"

class SerialMacCli: public Observer
{
public:
    SerialMacCli ( const char* portname );
    ~SerialMacCli ( );

    int InitSerialPort ();
    void DeInitSerialPort();
    void Update ( Event *event );

    int Run ( void );

private:
    struct sp_port_config *saved_port_config;
    const char *port_name;
    struct sp_port *port_context = NULL;
    struct sp_event_set *port_events = NULL;
    struct sf_serialmac_ctx *mac_context = NULL;

    int run = true;

    void Wait4UserInput ( void );
    void Wait4HalEvent ( int nano_nap );
};

#endif // SERIALMACCLI_H
