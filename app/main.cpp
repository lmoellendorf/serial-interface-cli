/*! ============================================================================
 *
 * @file main.cpp
 *
 * @date: 20.11.2014
 * @author: © by STACKFORCE, Heitersheim, Germany, http://www.stackforce.de
 * @author: Lars Möllendorf
 *
 * @brief  Sample header of the source code
 *
 * @version
 *
 =============================================================================*/

#include <iostream>
#include <stdio.h>
#include <thread>
#include <unistd.h>
#include <string.h>
extern "C"
{
#include <libserialport.h>
}
#include <stackforce_serial_mac_api.h>

#define SF_SERIAL_BAUDRATE 115200
#define SF_SERIAL_BITS 8
#define SF_SERIAL_STOPBITS 1
#define SF_SERIAL_FLOWCTRL SP_FLOWCONTROL_NONE
#define SF_SERIAL_INPUT_MAX_SIZE 255
#define FALSE 0
#define TRUE 1
#ifdef __linux__
#define SF_SERIAL_PORT_NAME "/dev/ttyUSB1"
#else
#ifdef _WIN32
#define SF_SERIAL_PORT_NAME "COM1"
#endif
#endif

using namespace std;

enum state
{
    START_FRAME,
    APPEND_FRAME,
};

struct app_ctx
{
    int run = TRUE;
    int status = START_FRAME;
    struct sp_port *port = NULL;
    struct sf_serial_mac_ctx *mac_ctx;
    size_t iBuffLen = 0;
    size_t oBuffRemains = 0;
    size_t oBuffLength = 0;
    char iBuff[SF_SERIAL_INPUT_MAX_SIZE];
    char oBuff[SF_SERIAL_INPUT_MAX_SIZE];
};

static struct app_ctx ctx;

void read_evt(const char *frameBuffer, size_t frameBufferLength);
void bufferRx_evt(const char *frameBuffer, size_t frameBufferLength);
void write_evt(size_t processed);
void bufferTx_evt(int processed);
void wait4userinput(void);
void wait4halEvent(enum sp_event event,
                   enum sf_serial_mac_return (*sf_serial_mac_halCb)(struct sf_serial_mac_ctx *ctx));
void wait4halTxEvent();
void wait4halRxEvent();

void read_evt(const char *frameBuffer, size_t frameBufferLength)
{
    if (frameBuffer && frameBufferLength)
    {
        if ('\n' == frameBuffer[0])
        {
            ctx.run = FALSE;
        }
        else
        {
            printf(":%s:%zd\n", frameBuffer, frameBufferLength);
        }
    }
}

void bufferRx_evt(const char *frameBuffer, size_t frameBufferLength)
{
    sf_serial_mac_rxFrame((struct sf_serial_mac_ctx *) ctx.mac_ctx, ctx.iBuff,
                          sizeof(ctx.iBuff));
}

void write_evt(size_t processed)
{
    ctx.status = START_FRAME;
}

void bufferTx_evt(size_t processed)
{
    ctx.oBuffRemains -= processed;
    thread userInputEventLoop(wait4userinput);
    userInputEventLoop.detach();
}

void wait4userinput(void)
{
    enum sf_serial_mac_return ret = SF_SERIAL_MAC_SUCCESS;
    string line = "";
    const size_t frmLength = 9;

    if(!ctx.oBuffRemains)
    {
        printf("Input text:\n");
        getline(cin, line);
        if (line.length() > 0)
        {
            ctx.oBuffLength = ctx.oBuffRemains = line.length();
            strncpy(ctx.oBuff,line.c_str(),sizeof ctx.oBuff);
        }
        else
        {
            /** Userinput was empty line -> STOP */
            ctx.run = FALSE;
            printf("Stop\n");
        }
    }
    if(ctx.oBuffRemains)
    {
        while((ret = sf_serial_mac_txFrame(ctx.mac_ctx, frmLength,
                                           ctx.oBuff + (ctx.oBuffLength - ctx.oBuffRemains),
                                           ctx.oBuffRemains)) != SF_SERIAL_MAC_SUCCESS)
        {
            printf("TX Error %i\nline: %s\nlength: %zd\n", ret,
                   ctx.oBuff + (ctx.oBuffLength - ctx.oBuffRemains), ctx.oBuffRemains);
            sleep(1);
        }
        //        switch (ctx.status)
//        {
//        case START_FRAME:
//            if((ret = sf_serial_mac_txFrameStart(ctx.mac_ctx,
//                                                 frmLength)) != SF_SERIAL_MAC_SUCCESS)
//            {
//                printf("Frame Error %i\n", ret);
//            }
//            ctx.status = APPEND_FRAME;
//        //break; omitted
//        case APPEND_FRAME:
//            while((ret = sf_serial_mac_txFrameAppend(ctx.mac_ctx,
//                         ctx.oBuff + (ctx.oBuffLength - ctx.oBuffRemains),
//                         ctx.oBuffRemains)) != SF_SERIAL_MAC_SUCCESS)
//            {
//                printf("TX Error %i\nline: %s\nlength: %zd\n", ret,
//                       ctx.oBuff + (ctx.oBuffLength - ctx.oBuffRemains), ctx.oBuffRemains);
//                sleep(1);
//            }
//            break;
//        default:
//            printf("Exception Error\n");
//            break;
//        }
    }
}

void wait4halTxEvent()
{
    wait4halEvent(SP_EVENT_TX_READY, sf_serial_mac_halTxCb);
}

void wait4halRxEvent()
{
    wait4halEvent(SP_EVENT_RX_READY, sf_serial_mac_halRxCb);
}
/**
 * This is a method stub to react on libserialport events. The plan is to
 * create 2 APIs for the MAC:
 * <ul>
 * <li>event driven</li>
 * <li>active polling</li>
 * </ul>
 */
void wait4halEvent(enum sp_event event,
                   enum sf_serial_mac_return (*sf_serial_mac_halCb)(struct sf_serial_mac_ctx *ctx))
{
    struct sp_event_set * portEventSet = NULL;
    unsigned int portEventMask = event;
    unsigned int timeout = 0;

    if (SP_OK <= sp_new_event_set(&portEventSet))
    {

        if (SP_OK
                <= sp_add_port_events(portEventSet, ctx.port,
                                      (enum sp_event) portEventMask))
        {
            while (SP_OK <= sp_wait(portEventSet, timeout))
            {
//                printf("masks: %X\nnumber of handles: %u\n",
//                        (unsigned int) *(portEventSet->masks),
//                        portEventSet->count);
                sf_serial_mac_halCb(ctx.mac_ctx);
                sleep(1);
            }
        }
        sp_free_event_set(portEventSet);
    }
    return;
}

int main(int argc, char **argv)
{

    uint8_t mac_ctx[sf_serial_mac_ctx_size()];
    ctx.mac_ctx = (struct sf_serial_mac_ctx*) mac_ctx;

    sp_return sp_ret = SP_OK;
//    struct sp_port **availablePorts = NULL;
    const char portname[] = SF_SERIAL_PORT_NAME;
    struct sp_port_config * savedPortConfig = NULL;

    //TODO: use sp_list_ports, look for valid port names and try to
    //connect them or let the user choose.
//    sp_ret = sp_list_ports(&availablePorts);
//    if (SP_OK > sp_ret)
//        return sp_ret;
//    if (NULL != availablePorts[0])
//    {
//        sp_ret = sp_copy_port(availablePorts[0], &port);
//    }
//    if (NULL != availablePorts)
//    {
//        sp_free_port_list(availablePorts);
//    }
//    if (SP_OK > sp_ret)
//        return sp_ret;

    sp_ret = sp_get_port_by_name(portname, &ctx.port);
    if (SP_OK > sp_ret || NULL == ctx.port)
    {
        printf("Port \"%s\" could not be found!\n", portname);
        return sp_ret;
    }

    sp_ret = sp_open(ctx.port, SP_MODE_READ_WRITE);
    if (SP_OK > sp_ret)
    {
        printf("Port \"%s\" could not be opened!\n", portname);
        return sp_ret;
    }

    /** Save current port configuration for later restoring */
    sp_ret = sp_new_config(&savedPortConfig);
    if (SP_OK > sp_ret)
    {
        printf("Config of port \"%s\" could not be saved! (Out of memory?)\n",
               portname);
        return sp_ret;
    }
    sp_ret = sp_get_config(ctx.port, savedPortConfig);
    if (SP_OK > sp_ret)
    {
        printf("Config of port \"%s\" could not be saved! (Read error?)\n",
               portname);
        return sp_ret;
    }

    sp_ret = sp_set_baudrate(ctx.port, SF_SERIAL_BAUDRATE);
    if (SP_OK > sp_ret)
    {
        printf("Could not set baudrate to %u on port \"%s\"!\n",
               SF_SERIAL_BAUDRATE, portname);
        return sp_ret;
    }

    sp_ret = sp_set_bits(ctx.port, SF_SERIAL_BITS);
    if (SP_OK > sp_ret)
    {
        printf("Could not set number of bits to %u on port \"%s\"!\n",
               SF_SERIAL_BITS, portname);
        return sp_ret;
    }

    sp_ret = sp_set_parity(ctx.port, SP_PARITY_NONE);
    if (SP_OK > sp_ret)
    {
        printf("Could not set parity to %u on port \"%s\"!\n",
               SP_PARITY_NONE, portname);
        return sp_ret;
    }

    sp_ret = sp_set_stopbits(ctx.port, SF_SERIAL_STOPBITS);
    if (SP_OK > sp_ret)
    {
        printf("Could not set stop-bits to %u on port \"%s\"!\n",
               SF_SERIAL_STOPBITS, portname);
        return sp_ret;
    }

    sp_ret = sp_set_flowcontrol(ctx.port, SF_SERIAL_FLOWCTRL);
    if (SP_OK > sp_ret)
    {
        printf("Could not set flow-control to %u on port \"%s\"!\n",
               SF_SERIAL_FLOWCTRL, portname);
        return sp_ret;
    }

    sf_serial_mac_init(ctx.mac_ctx,
                       (void *) ctx.port,
                       (SF_SERIAL_MAC_HAL_READ_FUNC) sp_nonblocking_read, (SF_SERIAL_MAC_HAL_READ_WAIT_FUNC) sp_input_waiting,
                       (SF_SERIAL_MAC_HAL_WRITE_FUNC) sp_nonblocking_write, read_evt, bufferRx_evt,
                       write_evt, bufferTx_evt);

    /** Start waiting for user input */
    thread userInputEventLoop(wait4userinput);
    userInputEventLoop.detach();

    /* Loop until the user quits */
    while (ctx.run)
    {
        sleep(1);
        sf_serial_mac_entry((struct sf_serial_mac_ctx *) ctx.mac_ctx);
    }

    if (NULL != ctx.port)
    {
        /** Restore previous port configuration */
        sp_ret = sp_set_config(ctx.port, savedPortConfig);
        sp_free_port(ctx.port);
    }
    return sp_ret;
}
