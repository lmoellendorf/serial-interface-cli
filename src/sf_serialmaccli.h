#ifndef SERIALMACCLI_H
#define SERIALMACCLI_H
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

#include <docopt.h>
#include <functional>
#include <string.h>
extern "C"
{
#include "libserialport.h"
#include "sf_serialmac.h"
}
#include "sf_observer.h"
#include "version.h"

namespace sf
{

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
    std::string port_name_object;
    const char *port_name = NULL;

    enum io_states {
        CLI,
        SERIAL
    };

    io_states cli_input_state;
    bool run;

    static int NonVerbose (const char *format, ...);
    int (*Verbose) (const char *format, ...);
    template<typename IfFunc, typename ElseFunc>
    void IfPayloadPassedAsParameter (
        IfFunc IfOperation, ElseFunc ElseOperation );
    int InitSerialPort ( );
    void DeInitSerialPort();
    void Quit();
    void CliInput ( void );
    void CliOutput ( void );
};

}
#endif // SERIALMACCLI_H
