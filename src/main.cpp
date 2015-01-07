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
using namespace std;
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "stackforce_serial_mac_api.h"

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyUSB0"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
//#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

volatile int STOP = FALSE;

struct serial_ctx
{
    int tty_fd;
    size_t buffLen;
    char *buff;
};

static struct serial_ctx ctx;

static ssize_t rx(int fd, const char *frameBuffer, size_t frameBufferLength);
static ssize_t tx(int fd, const char *frameBuffer, size_t frameBufferLength);
static void read_evt(const char *frameBuffer, size_t frameBufferLength);
static void write_evt(const char *frameBuffer, size_t frameBufferLength);
static struct apl_ctx* getCtxByFd(int fd);

static ssize_t rx(int fd, const char *frameBuffer, size_t frameBufferLength)
{
    return 0;
}

static ssize_t tx(int fd, const char *frameBuffer, size_t frameBufferLength)
{
    struct serial_ctx *ctx;
    if ((ctx = getCtxByFd(fd)))
    {
        if (ctx->buff == frameBuffer && ctx->buffLen == frameBufferLength)
        {
            return write(ctx->tty_fd, ctx->buff, ctx->buffLen);
        }
    }
    return -1;
}

static struct serial_ctx* getCtxByFd(int fd)
{
    if (fd == ctx.tty_fd)
    {
        return &ctx;
    }
    return NULL;
}

static void read_evt(const char *frameBuffer, size_t frameBufferLength)
{

}

static void write_evt(const char *frameBuffer, size_t frameBufferLength)
{

}

int main(int argc, char **argv)
{

    uint8_t mac_ctx[sf_serial_mac_ctx_size()];
//    struct sf_serial_mac_ctx mac_ctx;
    struct sf_serial_mac_ctx *mac_instance;
    struct termios tio;
    struct termios stdio;
    struct termios old_stdio;

    ctx.buffLen = 1;
    ctx.buff = new char[ctx.buffLen];
    bzero(&mac_ctx, sizeof(mac_ctx));

    /* save current port settings */
    tcgetattr(STDOUT_FILENO, &old_stdio);

    /* reset all settings to default */
    memset(&stdio, 0, sizeof(stdio));
    stdio.c_iflag = 0;
    stdio.c_oflag = 0;
    stdio.c_cflag = 0;
    stdio.c_lflag = 0;
    /* blocking read until 1 char received */
    stdio.c_cc[VMIN] = 1;
    /* inter-character timer */
    stdio.c_cc[VTIME] = 0;
    tcsetattr(STDOUT_FILENO, TCSANOW, &stdio);
    tcsetattr(STDOUT_FILENO, TCSAFLUSH, &stdio);
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);     // make the reads non-blocking

    memset(&tio, 0, sizeof(tio));
    tio.c_iflag = IGNPAR;
    tio.c_oflag = 0;
    tio.c_cflag = CS8 | CREAD | CLOCAL; // 8n1, see termios.h for more information
    /* set input mode (non-canonical, no echo,...) */
    tio.c_lflag = 0;
    /* blocking read until 1 char received */
    tio.c_cc[VMIN] = 1;
    /* inter-character timer */
    tio.c_cc[VTIME] = 5;

    ctx.tty_fd = open(MODEMDEVICE, O_RDWR | O_NONBLOCK);
    if (ctx.tty_fd < 0)
    {
        perror(MODEMDEVICE);
        exit(-1);
    }
    cfsetospeed(&tio, B115200);            // 115200 baud
    cfsetispeed(&tio, B115200);            // 115200 baud

    tcsetattr(ctx.tty_fd, TCSANOW, &tio);

    sf_serial_mac_init((struct sf_serial_mac_ctx *) mac_ctx,
            ctx.tty_fd, rx, tx, read_evt, write_evt);

    while (*(ctx.buff) != 'q')
    {
        if (read(ctx.tty_fd, ctx.buff, 1) > 0)
            write(STDOUT_FILENO, ctx.buff, 1); // if new data is available on the serial port, print it out
        if (read(STDIN_FILENO, ctx.buff, 1) > 0)
            sf_serial_mac_enqueFrame((struct sf_serial_mac_ctx *) mac_ctx,
                    ctx.buff, ctx.buffLen);
        //            write(ctx.tty_fd, ctx.buff, 1); // if new data is available on the console, send it to the serial port
        sf_serial_mac_entry((struct sf_serial_mac_ctx *) mac_ctx);
    }

    close(ctx.tty_fd);
    /* restore previous port settings */
    tcsetattr(STDOUT_FILENO, TCSANOW, &old_stdio);
    delete ctx.buff;
    return EXIT_SUCCESS;
}

