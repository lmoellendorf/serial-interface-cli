#ifndef SERIALMACCLI_H
#define SERIALMACCLI_H

#include <docopt.h>
#include <functional>
extern "C"
{
#include <libserialport.h>
}
#include "sf_observer.h"
#include "version.h"

class SerialMacCli: public Observer
{
public:
    SerialMacCli ( int argc, char **argv );
    ~SerialMacCli ( );

    int Run ( );
    void Update ( Event *event );

private:

    std::map<std::string, docopt::value> args;
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

    static int NonVerbose (const char *format, ...);
    int (*Verbose) (const char *format, ...);
    template<typename IfFunc, typename ElseFunc>
    void PayloadPassedAsParameter (
        IfFunc IfOperation, ElseFunc ElseOperation );
    int InitSerialPort ( );
    void DeInitSerialPort();
    void CliInput ( void );
    void CliOutput ( void );
};

#endif // SERIALMACCLI_H
