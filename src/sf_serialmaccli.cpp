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
 * @details See @code sfserialcli --help @endcode for details.
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

#include <iostream>
#include <thread>
#include <string.h>
#include <algorithm>
#include <docopt.h>
#include <condition_variable>

#include "sf_serialobserver.h"
#include "sf_serialmaccli.h"
#include "sf_serialmac.h"
#include "version.h"
#include "sf_stringhex.h"

#ifdef __WIN32__
#define CURRENT_SUPPLY_DEFAULT_PARAMETER " [default: d]"
#else
#define CURRENT_SUPPLY_DEFAULT_PARAMETER
#endif

#ifdef __WIN32_CROSS_BUILD__
#include "mingw.thread.h"
#include "mingw.mutex.h"
#include "mingw.condition_variable.h"
#endif

namespace sf {
    static const char USAGE[] =
    SERIALMACCLI_PRODUCT_NAME R"(
Copyright (C) 2017 )" SERIALMACCLI_PRODUCT_COMPANY R"( GmbH v)" SERIALMACCLI_VERSION R"(

      Usage:
      )" SERIALMACCLI_PROGRAM_NAME R"( [options] [<payload> ...]

      Options:
      -h, --help                                  Show this screen.
      -v, --version                               Show version.
      -d <device>, --device=<device>              Serial port to use (e.g. "/dev/tty0", "/dev/ttyUSB0" or "COM1").
                                                  If none is given, the first available port is chosen.
      -b <baudrate>, --baudrate=<baudrate>        Baud rate [default: 115200].
      -D (5-8), --data-bits=(5-8)                 Data bits [default: 8].
      -P (n|o|e|s|m), --parity-bit=(n|o|e|s|m)    Parity bit mode [default: n]:
                                                  n: None
                                                  o: Odd
                                                  e: Even
                                                  s: Space
                                                  m: Mark
      -S (1|2), --stop-bits=(1|2)                 Stop bits [default: 1].
      -F (n|x|r|d), --flow-control=(n|x|r|d)      Flow control mode [default: n]:
                                                  n: None
                                                  x: XON/XOFF
                                                  r: RTS/CTS
                                                  d: DTR/DSR
      -C (n|d|r|dr|rd), --current=(n|d|r|dr|rd)   Current supply)"
      CURRENT_SUPPLY_DEFAULT_PARAMETER R"(:
                                                  n: None
                                                  d: Power DTR
                                                  r: Power RTS
                                                  dr or rd: Power both
      -X (i|o|io|oi), --xon-xoff=(i|o|io|oi)      XON/XOFF flow control behaviour:
                                                  i: Enabled for input only
                                                  o: Enabled for output only
                                                  io or oi: Enabled for input and output
      -I (d|r|c|s|x), --ignore=(d|r|c|s|x)        ignore configuration options:
                                                  Do not configure DTR
                                                  Do not configure RTS
                                                  Do not configure CTS
                                                  Do not configure DSR
                                                  Do not configure XON/XOFF
      -t, --text                                  Send and receive plain text instead of converting it to binary values first.
      -s <delimiters>, --delimiters=<delimiters>  String delimiter(s) [default: ,;.: ] <- The last default is a whitespace!
                                                  Will split the string at the given delimiters before converting them to binary values.
      -V, --verbose                               Verbosive debug information on stderr.
      )";

SerialMacCli::SerialMacCli(int argc, char **argv) : SerialObserver() {
    run = true;
    docopt::value value;
    exitStatus = EXIT_SUCCESS;
    args = docopt::docopt ( USAGE,
    { argv + 1, argv + argc },
    true,               // show help if requested
    SERIALMACCLI_PRODUCT_NAME R"(
Copyright (C) 2017 )" SERIALMACCLI_PRODUCT_COMPANY R"( GmbH
CLI v)" SERIALMACCLI_VERSION R"(
MAC v)" SERIALMAC_VERSION, false ); // version string

    value = args.at ( "--verbose" );
    if(value && value.isBool()) {
        if( value.asBool()) {
            Verbose = std::printf;
        }
        else {
            Verbose = NonVerbose;
        }
    }

    value = args.at("<payload>");
    if(value && value.isStringList() && value.asStringList().size() != 0) {
        interactive = false;
    }
    else {
        interactive = true;
    }

    /* for debugging docopt
    for ( auto const& arg : args )
    {
        std::cout << arg.first << ": " <<  arg.second << std::endl;
    }
    */
}

SerialMacCli::~SerialMacCli() {
    DetachSerial();
}

/**
 * This is a dummy function which is used instead of printf in non-verbose
 * mode.
 */
int SerialMacCli::NonVerbose (const char *format, ...) {
  return strlen(format);
}

/**
 * Initialize the serial port.
 */
SerialObserver::SerialObserverStatus SerialMacCli::InitSerialPort() {
    docopt::value value;

    serialPortConfig = new SerialPortConfig(args.at( "--device" ).asString());
    serialPortConfig->SetMode(SerialPortConfig::PortMode::PORTMODE_READWRITE);
    serialPortConfig->SetBaudRate(args.at("--baudrate").asLong());
    serialPortConfig->SetDataBits(args.at("--data-bits").asLong());
    serialPortConfig->SetStopBits(args.at("--stop-bits").asLong());

    value = args.at("--parity-bit");
    if(value && value.isString()) {
        switch(value.asString().at(0)) {
        case 'n':
            serialPortConfig->SetParityBit(SerialPortConfig::ParityBit::PARBIT_NONE);
            break;
        case 'o':
            serialPortConfig->SetParityBit(SerialPortConfig::ParityBit::PARBIT_ODD);
            break;
        case 'e':
            serialPortConfig->SetParityBit(SerialPortConfig::ParityBit::PARBIT_EVEN);
            break;
        case 's':
            serialPortConfig->SetParityBit(SerialPortConfig::ParityBit::PARBIT_SPACE);
            break;
        case 'm':
            serialPortConfig->SetParityBit(SerialPortConfig::ParityBit::PARBIT_MARK);
            break;
        }
    }

    value = args.at("--flow-control");
    if(value && value.isString()) {
        switch( value.asString().at(0)) {
        case 'n':
            serialPortConfig->SetFlowCtrl(SerialPortConfig::FlowCtrl::FLOWCTRL_NONE);
            break;
        case 'x':
            serialPortConfig->SetFlowCtrl(SerialPortConfig::FlowCtrl::FLOWCTRL_XONXOFF);
            break;
        case 'r':
            serialPortConfig->SetFlowCtrl(SerialPortConfig::FlowCtrl::FLOWCTRL_RTSCTS);
            break;
        case 'd':
            serialPortConfig->SetFlowCtrl(SerialPortConfig::FlowCtrl::FLOWCTRL_DTRDSR);
            break;
        }
    }

    value = args.at("--xon-xoff");
    if(value && value.isString()) {
        if(value.asString().length() > 1
            && (value.asString() == "io"
                || value.asString() == "oi")) {
            serialPortConfig->SetXonXoffBehaviour(SerialPortConfig::XonXoffBehaviour::XONXOFF_INOUT);
        }

        switch(value.asString().at(0)) {
        case 'i':
            serialPortConfig->SetXonXoffBehaviour(SerialPortConfig::XonXoffBehaviour::XONXOFF_IN);
            break;
        case 'o':
            serialPortConfig->SetXonXoffBehaviour(SerialPortConfig::XonXoffBehaviour::XONXOFF_OUT);
            break;
        }
    }

    value = args.at ( "--current" );
    if(value && value.isString()) {
        if(value.asString().length() > 1
            && (value.asString() == "dr"
                || value.asString() == "rd")) {
            serialPortConfig->SetRts(SerialPortConfig::RTS::RTS_ON);
            serialPortConfig->SetDtr(SerialPortConfig::DTR::DTR_ON);
        }

        switch (value.asString().at(0)) {
        case 'd':
            serialPortConfig->SetRts(SerialPortConfig::RTS::RTS_OFF);
            serialPortConfig->SetDtr(SerialPortConfig::DTR::DTR_ON);
            break;
        case 'r':
            serialPortConfig->SetRts(SerialPortConfig::RTS::RTS_ON);
            serialPortConfig->SetDtr(SerialPortConfig::DTR::DTR_OFF);
            break;
        case 'n':
            serialPortConfig->SetRts(SerialPortConfig::RTS::RTS_OFF);
            serialPortConfig->SetDtr(SerialPortConfig::DTR::DTR_OFF);
            break;
        }
    }

    value = args.at ("--ignore");
    if(value && value.isString()) {
        switch(value.asString().at(0)) {
        case 'd':
            serialPortConfig->SetDtr(SerialPortConfig::DTR::DTR_INVALID);
            break;
        case 'r':
            serialPortConfig->SetRts(SerialPortConfig::RTS::RTS_INVALID);
            break;
        case 'c':
            serialPortConfig->SetCts(SerialPortConfig::CTS::CTS_INVALID);
            break;
        case 's':
            serialPortConfig->SetDsr(SerialPortConfig::DSR::DSR_INVALID);
            break;
        case 'x':
            serialPortConfig->SetXonXoffBehaviour(SerialPortConfig::XonXoffBehaviour::XONXOFF_INVALID);
            break;
        }
    }

    return AttachSerial(serialPortConfig);
}

void SerialMacCli::Quit() {
    Verbose ( "Quitting.\n" );
    run = false;
    running.notify_one();
}

/**
 * Using a template allows us to ignore the differences between functors,
 * function pointers and lambda
 */
template<typename IfFunc, typename ElseFunc>
/**
 * To avoid code rendundancy this function executes the IfOperation if
 * payload has been passed as parameter and the ElseOperation otherwise.
 */
void SerialMacCli::IfPayloadPassedAsParameter(IfFunc IfOperation, ElseFunc ElseOperation) {
    docopt::value value = args.at("<payload>");

    if(!interactive) {
        return IfOperation(value);
    }

    return ElseOperation();
}


void SerialMacCli::CliInput(void) {
    std::string line = "";
    docopt::value value;
    std::string delimiters;
    char *output_buffer = NULL;
    int output_buffer_length = 0;
    std::vector<uint8_t> payload;

    /** Repeat until the user stops you */
    while(run) {
        switch(ioState) {
            case IoState::CLI:
                /** If payload is passed as parameter ... */
                IfPayloadPassedAsParameter(
                /** ... this lambda function is executed ... */
                [&line, this](docopt::value value) {
                    std::vector <std::string> line_as_list = value.asStringList();
                    std::for_each(line_as_list.begin(), line_as_list.end(),
                                    /**
                                        * A lambda within a lambda! But this is how
                                        * for_each works
                                        */
                                    [&line](std::string& word)
                    {
                        return line+=word;
                    });
                },
                /** ... else this lambda function is executed */
                [&line, this]() {
                    Verbose ("Input text:\n");
                    std::getline(std::cin, line);
                });

                if(line.length() > 0) {
                    /**
                    * We cannot simply pass a pointer to line.c_str() or
                    * hex_binaries[0] here because their memory will be destroyed
                    * as soon as we leave the scope of this function and the serial
                    * MAC processes the memory asynchronously.
                    * Therefore we allocate memory and copy the content.
                    */
                    value = args.at("--text");
                    if(value && value.isBool() && value.asBool()) {
                        output_buffer_length = line.length() + 1/* for the terminating '\0' */;
                        /** Will be freed in Update() when TX has been completed. */
                        output_buffer = ( char* ) std::malloc ( output_buffer_length );
                        strncpy ( output_buffer, line.c_str(), output_buffer_length );
                    }
                    else {
                        value = args.at("--delimiters");
                        if(value && value.isString()) {
                            delimiters = value.asString();
                        }
                        /**
                        * In C++ there is no easy way to separate declaration and
                        * initialization of objects
                        */
                        StringHex hex(delimiters);
                        std::vector<uint8_t> hex_binaries;
                        hex.HexStringToBinary(line, hex_binaries);
                        output_buffer_length = hex_binaries.size();
                        /**
                        * On invalid input the length is 0 and we are finished for
                        * the moment
                        */
                        if(!output_buffer_length)
                        Quit();
                        /** Will be freed in Update() when TX has been completed. */
                        output_buffer = (char*)std::malloc(output_buffer_length);
                        std::copy(hex_binaries.begin(), hex_binaries.end(), output_buffer);
                    }

                    payload.assign(output_buffer, output_buffer + output_buffer_length);
                    ioState = IoState::SERIAL;
                    SendSerial(payload);
                }
                else {
                    Quit();
                }

            break;

        case IoState::SERIAL: //nothing to do
          break;
        }
    }
}

int SerialMacCli::Run() {
    if(InitSerialPort() != SerialObserverStatus::ATTACH_OK) {
        std::cerr << "Could not initialize serial port " << serialPortConfig->GetPortName() << std::endl;
        return EXIT_FAILURE;
    }

    /** Start waiting for CLI input */
    ioState = IoState::CLI;
    std::thread threadCliInput(&SerialMacCli::CliInput, this);
    threadCliInput.detach();

    std::unique_lock<std::mutex> lockRunning(runningMutex);
    running.wait(lockRunning);

    return exitStatus;
}

void SerialMacCli::Update(Event* event) {
    uint8_t *bufferContent = NULL;
    size_t bufferSize;
    std::vector<uint8_t> payload;
    docopt::value value;

    switch(event->GetIdentifier()) {
        case SerialHandler::SERIAL_READ_FRAME_EVENT:

            bufferSize = event->GetDetails((void**)&bufferContent);
            payload.assign(bufferContent, bufferContent+bufferSize);

            /** Check if a valid frame has been received */
            if(bufferSize) {
                value = args.at ( "--text" );
                if(value && value.isBool() && value.asBool()) {
                    if('\n' == bufferContent[0]) {
                        Quit();
                    }
                    else {
                        std::printf("%s\n", bufferContent);
                    }
                }
                else {
                    std::string delimiters;
                    value = args.at ( "--delimiters" );
                    if(value && value.isString()) {
                        delimiters = value.asString();
                    }
                    StringHex hex(delimiters);
                    std::string hex_string;
                    std::vector<uint8_t> hex_binaries(bufferContent, bufferContent + bufferSize);
                    hex.BinaryToHexString(hex_binaries, hex_string);
                    std::cout << hex_string << std::endl;

                }
                Verbose("Length:\n%zd\n", bufferSize);
            }

            if(!interactive) {
                Quit();
            }
            else {
                ioState = IoState::CLI;
            }
            break;

        case SerialHandler::SERIAL_READ_BUFFER_EVENT:
            break;

        case SerialHandler::SERIAL_WRITE_FRAME_EVENT:
            break;

        case SerialHandler::SERIAL_WRITE_BUFFER_EVENT:
            break;

        case SerialHandler::SERIAL_READ_SYNC_BYTE_EVENT:
            break;

        case SerialHandler::SERIAL_CONNECTION_ERROR:

            bufferSize = event->GetDetails((void**)&bufferContent);
            std::cerr << "DeviceHandler::Update -> got SERIAL_CONNECTION_ERROR" << std::endl;
            exitStatus = EXIT_FAILURE;
            this->Quit();
            break;

        case SerialHandler::SERIAL_MAC_ERROR_CRC:
            std::cerr << "DeviceHandler::Update -> got SERIAL_MAC_ERROR_CRC" << std::endl;
            if(!interactive) {
                Quit();
            }
            else {
                ioState = IoState::CLI;
            }
            break;

        case SerialHandler::SERIAL_MAC_ERROR_SYNC_BYTE:
            std::cerr << "DeviceHandler::Update -> got SERIAL_MAC_ERROR_SYNC_BYTE" << std::endl;
            if(!interactive) {
                Quit();
            }
            else {
                ioState = IoState::CLI;
            }
            break;

        default:
            std::cerr << "DeviceHandler::Update -> got unhandled event: " << event->GetIdentifier() << std::endl;
            break;
    }
}

}
