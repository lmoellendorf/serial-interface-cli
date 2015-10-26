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

    int Init ( int argc, char **argv );
    int Run ( );
    void Update ( Event *event );

private:

    struct sp_port_config *port_config_backup;
    struct sp_port_config *port_config_new;
    struct sp_port *port_context = NULL;
    struct sp_event_set *port_rx_event = NULL;
    struct sp_event_set *port_tx_event = NULL;
    struct sf_serialmac_ctx *mac_context = NULL;
    const char *port_name = NULL;

    enum io_states {
        CLI,
        SERIAL,
        QUIT
    };

    io_states cli_input_state;
    io_states cli_output_state;

    int InitSerialPort ( std::map<std::string, docopt::value> args );
    void DeInitSerialPort();
    void CliInput ( void );
    void CliOutput ( void );
    int (*verbose) (const char *format, ...);
};

#endif // SERIALMACCLI_H
