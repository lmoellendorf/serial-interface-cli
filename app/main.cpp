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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <thread>
#include <unistd.h>
#include <sys/signal.h>
#include <fcntl.h>
extern "C"
{
#include <libserialport.h>
}
#include <stackforce_serial_mac_api.h>

//#include "stackforce_serial_hal.h"

//#define BAUDRATE B115200
//#define MODEMDEVICE "/dev/ttyUSB0"
//#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
//#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

using namespace std;

volatile int STOP = FALSE;
//int wait_flag = TRUE;
volatile int wait4MAC = TRUE;

//struct apl_ctx
//{
//    int wait4hal = TRUE;
//    int tty_fd;
//    size_t iBuffLen;
//    char *iBuff;
//    size_t oBuffLen;
//    const char *oBuff;
//};
//
//static struct apl_ctx ctx;

//static ssize_t rx(int fd, const char *frameBuffer, size_t frameBufferLength);
//static ssize_t tx(int fd, const char *frameBuffer, size_t frameBufferLength);
static void read_evt(const char *frameBuffer, size_t frameBufferLength);
static void write_evt(const char *frameBuffer, size_t frameBufferLength);
//static struct apl_ctx* getCtxByFd(int fd);
///* definition of signal handler */
//static void signal_handler_IO(int status);

//static ssize_t rx(int fd, const char *frameBuffer, size_t frameBufferLength)
//{
//    struct apl_ctx *ctx;
//    if ((ctx = getCtxByFd(fd)))
//    {
////    ctx.tty_fd, ctx.iBuff,
//    }
//    return 0;
//}
//
//static ssize_t tx(int fd, const char *frameBuffer, size_t frameBufferLength)
//{
//    struct apl_ctx *ctx;
//    if ((ctx = getCtxByFd(fd)))
//    {
//        if (ctx->oBuff == frameBuffer && ctx->oBuffLen == frameBufferLength)
//        {
//            return write(ctx->tty_fd, ctx->oBuff, ctx->oBuffLen);
//        }
//    }
//    return -1;
//}
//
//static struct apl_ctx* getCtxByFd(int fd)
//{
//    if (fd == ctx.tty_fd)
//    {
//        return &ctx;
//    }
//    return NULL;
//}

static void read_evt(const char *frameBuffer, size_t frameBufferLength)
{

}

static void write_evt(const char *frameBuffer, size_t frameBufferLength)
{
    wait4MAC = FALSE;
}

void userinput(struct sf_serial_mac_ctx *mac_ctx)
{
    while (TRUE)
    {
        string line;
        getline(cin, line);
        line += "\n";
//        ctx->oBuffLen = line.length();
//        ctx->oBuff = line.c_str();
//        ctx->wait4hal = TRUE;
        wait4MAC = TRUE;
        sf_serial_mac_enqueFrame(mac_ctx, line.c_str(), line.length());
        while (wait4MAC)
            ;
    }
}

///***************************************************************************
// * signal handler. sets wait_flag to FALSE, to indicate above loop that     *
// * characters have been received.                                           *
// ***************************************************************************/
//
//void signal_handler_IO(int status)
//{
//    printf("received SIGIO signal.\n");
//    wait_flag = FALSE;
//}
//
int main(int argc, char **argv)
{

    uint8_t mac_ctx[sf_serial_mac_ctx_size()];

    struct sp_port **port;

    if(SP_OK != sp_list_ports(&port))
    {
        return EXIT_FAILURE;
    }

//    stackforce::serial::Hal serial;
//    struct termios tio;
//    struct termios oldtio;
//    /* definition of signal action */
//    struct sigaction saio;
//
//    ctx.iBuffLen = 5;
//    ctx.iBuff = new char[ctx.iBuffLen];

//    sf_serial_mac_init((struct sf_serial_mac_ctx *) mac_ctx, serial.getTtyFd(),
//            &stackforce::serial::Hal::rx, &stackforce::serial::Hal::tx,
//            read_evt, write_evt);

thread get(userinput, (struct sf_serial_mac_ctx*) &mac_ctx);
get.detach();

//    while (*(ctx.iBuff) != 'q')
//    {
//        if (read(ctx.tty_fd, ctx.iBuff, 1) > 0)
//            write(STDOUT_FILENO, ctx.iBuff, 1); // if new data is available on the serial port, print it out
//        sf_serial_mac_entry((struct sf_serial_mac_ctx *) mac_ctx);
//    }
/* loop while waiting for input. normally we would do something
 useful here */
while (STOP == FALSE)
{
    ;
//        printf(".\n");
//        usleep(100000);
    /* after receiving SIGIO, wait_flag = FALSE, input is available
     and can be read */
//        if (wait_flag == FALSE)
//        {
//            ctx.iBuffLen = read(ctx.tty_fd, ctx.iBuff, 255);
//            ctx.iBuff[ctx.iBuffLen] = 0;
//            printf(":%s:%zd\n", ctx.iBuff, ctx.iBuffLen);
//            if (ctx.iBuffLen == 1)
//                STOP = TRUE; /* stop loop if only a CR was input */
//            wait_flag = TRUE; /* wait for new input */
//        }
//    sf_serial_mac_entry((struct sf_serial_mac_ctx *) mac_ctx);
}

//    /* restore old port settings */
//    tcsetattr(ctx.tty_fd, TCSANOW, &oldtio);
//    close(ctx.tty_fd);
//    delete ctx.iBuff;
return EXIT_SUCCESS;
}
