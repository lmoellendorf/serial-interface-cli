#ifndef SERIALMACCLI_H
#define SERIALMACCLI_H

#include <docopt.h>
extern "C"
{
#include <libserialport.h>
}
#include "sf_observer.h"
#include "version.h"

class SerialMacCli: public Observer
{
public:
    SerialMacCli ( );
    ~SerialMacCli ( );

    void Update ( Event *event );

    int Run (  int argc, char **argv );

private:

    struct sp_port_config *port_config_backup;
    struct sp_port_config *port_config_new;
    struct sp_port *port_context = NULL;
    struct sp_event_set *port_events = NULL;
    struct sf_serialmac_ctx *mac_context = NULL;

    int run = true;

    int InitSerialPort ( std::map<std::string, docopt::value> args );
    void DeInitSerialPort();
    void Wait4UserInput ( void );
    void Wait4HalEvent ( int nano_nap );
};

#endif // SERIALMACCLI_H
