#ifndef _SF_SERIALMACCLI_H_
#define _SF_SERIALMACCLI_H_
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
 * @author     Lars Möllendorf
 * @author     Adrian Antonana
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
#include "sf_serialobserver.h"
#include "version.h"

namespace sf {

    class SerialMacCli: public SerialObserver {

        public:
            SerialMacCli(int argc, char **argv);
            ~SerialMacCli();

            int Run();
            void Update(Event *event);

        private:
            std::map<std::string, docopt::value> args;
            SerialPortConfig *serialPortConfig = nullptr;

            enum class IoState {
                CLI,
                SERIAL
            };

            IoState ioState;
            bool run;

            static int NonVerbose(const char *format, ...);
            int (*Verbose) (const char *format, ...);
            template<typename IfFunc, typename ElseFunc>
            void IfPayloadPassedAsParameter (
                IfFunc IfOperation, ElseFunc ElseOperation );
            SerialObserverStatus InitSerialPort();
            void DeInitSerialPort();
            void Quit();
            void CliInput(void);
    };

}
#endif // _SF_SERIALMACCLI_H_
