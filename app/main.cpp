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
extern "C"
{
#include <libserialport.h>
}
#include <stackforce_serial_mac_api.h>

#define SF_SERIAL_BAUDRATE 115200
#define SF_SERIAL_INPUT_MAX_SIZE 255
#define FALSE 0
#define TRUE 1
#ifdef __linux__
#define SF_SERIAL_PORT_NAME "/dev/ttyUSB0"
#else
#ifdef _WIN32
#define SF_SERIAL_PORT_NAME "COM1"
#endif
#endif

using namespace std;

struct app_ctx
{
    int run = TRUE;
    struct sp_port *port = NULL;
    struct sf_serial_mac_ctx *mac_ctx;
    size_t iBuffLen = 0;
    char iBuff[SF_SERIAL_INPUT_MAX_SIZE];
};

static struct app_ctx ctx;

void read_evt(const char *frameBuffer, size_t frameBufferLength);
void write_evt(const char *frameBuffer, size_t frameBufferLength);
void wait4userinput();
void wait4halEvent(enum sp_event event,
        void* (*sf_serial_mac_halCb)(struct sf_serial_mac_ctx *ctx));
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
            sf_serial_mac_rxFrame((struct sf_serial_mac_ctx *) ctx.mac_ctx,
                    ctx.iBuff, sizeof(ctx.iBuff));
        }
    }
}

void write_evt(const char *frameBuffer, size_t frameBufferLength)
{
    thread userInputEventLoop(wait4userinput);
    userInputEventLoop.detach();
}

void wait4userinput()
{
    string line;
    getline(cin, line);
    if (line.length() > 0)
    {
        line += "\n";
        sf_serial_mac_txFrame(ctx.mac_ctx, line.c_str(), line.length());
    }
    else
    {
        /** Userinput was empty line -> STOP */
        ctx.run = FALSE;
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
        void* (*sf_serial_mac_halCb)(struct sf_serial_mac_ctx *ctx))
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
    if (SP_OK > sp_ret)
        return sp_ret;

    if (NULL == ctx.port)
        return EXIT_FAILURE;

    sp_ret = sp_open(ctx.port, SP_MODE_READ_WRITE);
    if (SP_OK > sp_ret)
        return sp_ret;

    /** Save current port configuration for later restoring */
    sp_ret = sp_new_config(&savedPortConfig);
    if (SP_OK > sp_ret)
        return sp_ret;
    sp_ret = sp_get_config(ctx.port, savedPortConfig);
    if (SP_OK > sp_ret)
        return sp_ret;

    sp_ret = sp_set_baudrate(ctx.port, SF_SERIAL_BAUDRATE);
    if (SP_OK > sp_ret)
        return sp_ret;

    sf_serial_mac_init((struct sf_serial_mac_ctx *) ctx.mac_ctx,
            (void *) ctx.port,
            (SF_SERIAL_MAC_HAL_READ_FUNC) sp_nonblocking_read,
            (SF_SERIAL_MAC_HAL_WRITE_FUNC) sp_nonblocking_write, read_evt,
            write_evt);

    sf_serial_mac_rxFrame((struct sf_serial_mac_ctx *) ctx.mac_ctx, ctx.iBuff,
            sizeof(ctx.iBuff));

    /** Start waiting for user input */
    write_evt(NULL, 0);

//    thread txEventLoop(wait4halTxEvent);
//    txEventLoop.detach();
//
//    thread rxEventLoop(wait4halRxEvent);
//    rxEventLoop.detach();

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
