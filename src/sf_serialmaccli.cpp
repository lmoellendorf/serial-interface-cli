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
      --no-inverted-length                        Disables the inverted length field in MAC header (For MAC versions <= 2.0.0).
      -V, --verbose                               Verbosive debug information on stderr.
      )";

SerialMacCli::SerialMacCli(int argc, char **argv) : SerialObserver() {

    docopt::value value;
    exitStatus = ExitStatus::EXIT_OK;
    args = docopt::docopt ( USAGE,
    { argv + 1, argv + argc },
    true,               // show help if requested
    SERIALMACCLI_PRODUCT_NAME R"(
Copyright (C) 2017 )" SERIALMACCLI_PRODUCT_COMPANY R"( GmbH
CLI v)" SERIALMACCLI_VERSION R"(
MAC v)" SERIALMAC_VERSION, false ); // version string

    value = args.at ( "--verbose" );
    if(value && value.isBool()) {
        if(value.asBool()) {
            Verbose = std::fprintf;
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

    value = args.at("--no-inverted-length");
    noInvertedLengthField = value.asBool();

    value = args.at("--text");
    if(value && value.isBool()) {
        textMode = value.asBool();
    }

    value = args.at("--delimiters");
    if(value && value.isString()) {
        delimiters = value.asString();
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
int SerialMacCli::NonVerbose(FILE *stream, const char *format, ...) {
  return strlen(format);
}

/**
 * Initialize the serial port.
 */
SerialObserver::SerialObserverStatus SerialMacCli::InitSerialPort() {
    docopt::value value;

    serialMACConfig = new SerialMACConfig();
    if(noInvertedLengthField) {
        serialMACConfig->SetLengthFieldType(SerialMACConfig::LengthField::LENGTHFIELD_SIMPLE);
    }
    else {
        serialMACConfig->SetLengthFieldType(SerialMACConfig::LengthField::LENGTHFIELD_EXTENDED);
    }


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

    return AttachSerial(serialPortConfig, serialMACConfig);
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
    char *outputBuffer = NULL;
    int outputBufferLength = 0;
    std::vector<uint8_t> payload;
    std::vector<uint8_t> hexBinaries;
    bool run = true;

    /** Repeat until the user stops you */
    while(run) {
        /** If payload is passed as parameter ... */
        IfPayloadPassedAsParameter(
        /** ... this lambda function is executed ... */
        [&line, this](docopt::value value) {
            std::vector <std::string> line_as_list = value.asStringList();
            std::for_each(line_as_list.begin(), line_as_list.end(), [&line](std::string& word) {
                return line+=word;
            });
        },
        /** ... else this lambda function is executed */
        [&line, this]() {
            std::getline(std::cin, line);
        });

        if(line.length() > 0) {

            if(textMode) {
                outputBufferLength = line.length() + 1;
                outputBuffer = (char*)std::malloc(outputBufferLength);
                strncpy(outputBuffer, line.c_str(), outputBufferLength);
                payload.assign(outputBuffer, outputBuffer + outputBufferLength);
                std::free(outputBuffer);
            }
            else {
                StringHex hex(delimiters);
                hex.HexStringToBinary(line, hexBinaries);

                if(!hexBinaries.size()) {
                    run = false;
                    break;
                }
                else {
                    payload = hexBinaries;
                }

                hexBinaries.clear();
            }

            SendSerial(payload);

            if(!interactive) {
                run = false;
            }
        }
        else {
            run = false;
        }
    }

    userInput.notify_one();
}

SerialMacCli::ExitStatus SerialMacCli::Run() {

    std::unique_lock<std::mutex> lockConfirm(confirmMutex);
    std::unique_lock<std::mutex> lockInput(inputMutex);
    std::chrono::seconds confirmTimeout(respTimeoutSecs);

    // initialize serial port
    if(InitSerialPort() != SerialObserverStatus::OBSERVER_OK) {
        std::cerr << "Could not initialize serial port " << serialPortConfig->GetPortName() << std::endl;
        return ExitStatus::EXIT_ERROR;
    }

    // spawn user input thread
    std::thread threadCliInput(&SerialMacCli::CliInput, this);
    threadCliInput.detach();

    if(interactive) { // in interactive mode block while expecting user input
        userInput.wait(lockInput);
    }
    else { // in non interactive mode block for a confirmation to be received within a timeout

        if(confirmation.wait_for(lockConfirm, confirmTimeout) == std::cv_status::timeout) {
            return SerialMacCli::ExitStatus::EXIT_TIMEOUT;
        }
    }

    return exitStatus;
}

void SerialMacCli::Update(Event* event) {
    uint8_t *bufferContent = NULL;
    size_t bufferSize;
    std::vector<uint8_t> payload;
    docopt::value value;

    switch(event->GetIdentifier()) {
        case SerialHandler::SERIAL_READ_FRAME_EVENT:

            Verbose(stderr, ":: SERIAL_READ_FRAME_EVENT\n");

            bufferSize = event->GetDetails((void**)&bufferContent);
            payload.assign(bufferContent, bufferContent+bufferSize);

            /** Check if a valid frame has been received */
            if(bufferSize) {

                Verbose(stderr, "Payload: ");

                if(textMode) {
                    if('\n' == bufferContent[0]) {
                        userInput.notify_one();
                    }
                    else {
                        std::printf("%s\n", bufferContent);
                    }
                }
                else {
                    StringHex hex(delimiters);
                    std::string hex_string;
                    std::vector<uint8_t> hex_binaries(bufferContent, bufferContent + bufferSize);
                    hex.BinaryToHexString(hex_binaries, hex_string);
                    std::cout << hex_string << std::endl;

                }
                Verbose(stderr, "Length: %zd\n", bufferSize);
            }

            if(!interactive) {
                confirmation.notify_one();
            }
            break;

        case SerialHandler::SERIAL_READ_BUFFER_EVENT:
            Verbose(stderr, ":: SERIAL_READ_BUFFER_EVENT\n");
            break;

        case SerialHandler::SERIAL_WRITE_FRAME_EVENT:
            Verbose(stderr, ":: SERIAL_WRITE_FRAME_EVENT\n");
            break;

        case SerialHandler::SERIAL_WRITE_BUFFER_EVENT:
            Verbose(stderr, ":: SERIAL_WRITE_BUFFER_EVENT\n");
            break;

        case SerialHandler::SERIAL_READ_SYNC_BYTE_EVENT:
            Verbose(stderr, ":: SERIAL_READ_SYNC_BYTE_EVENT\n");
            break;

        case SerialHandler::SERIAL_CONNECTION_ERROR:

            bufferSize = event->GetDetails((void**)&bufferContent);
            std::cerr << ":: SERIAL_CONNECTION_ERROR" << std::endl;
            exitStatus = ExitStatus::EXIT_ERROR;
            userInput.notify_one();
            confirmation.notify_one();
            break;

        case SerialHandler::SERIAL_MAC_ERROR_CRC:
            std::cerr << ":: SERIAL_MAC_ERROR_CRC" << std::endl;
            if(!interactive) {
                confirmation.notify_one();
            }
            break;

        case SerialHandler::SERIAL_MAC_ERROR_SYNC_BYTE:
            std::cerr << ":: SERIAL_MAC_ERROR_SYNC_BYTE" << std::endl;
            if(!interactive) {
                confirmation.notify_one();
            }
            break;

        default:
            Verbose(stderr, ":: UNHANDLED EVENT: %i\n", event->GetIdentifier());
            break;
    }
}

}
