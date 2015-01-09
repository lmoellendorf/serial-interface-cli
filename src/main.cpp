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
#include <thread>
#include <unistd.h>

#include "stackforce_serial_mac_api.h"

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyUSB0"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
//#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

volatile int STOP = FALSE;

struct apl_ctx
{
    int wait4hal = TRUE;
    int tty_fd;
    size_t iBuffLen;
    char *iBuff;
    size_t oBuffLen;
    const char *oBuff;
};

static struct apl_ctx ctx;

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
    struct apl_ctx *ctx;
    if ((ctx = getCtxByFd(fd)))
    {
        if (ctx->oBuff == frameBuffer && ctx->oBuffLen == frameBufferLength)
        {
            return write(ctx->tty_fd, ctx->oBuff, ctx->oBuffLen);
        }
    }
    return -1;
}

static struct apl_ctx* getCtxByFd(int fd)
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
    ctx.wait4hal = FALSE;
}

void userinput(struct apl_ctx *ctx, struct sf_serial_mac_ctx *mac_ctx)
{
    while (TRUE)
    {
        string line;
        getline(cin, line);
        line += "\n";
        ctx->oBuffLen = line.length();
        ctx->oBuff = line.c_str();
        sf_serial_mac_enqueFrame(mac_ctx, ctx->oBuff, ctx->oBuffLen);
        while (ctx->wait4hal)
            ;
    }
}

int main(int argc, char **argv)
{

    uint8_t mac_ctx[sf_serial_mac_ctx_size()];
    struct termios tio;

    ctx.iBuffLen = 5;
    ctx.iBuff = new char[ctx.iBuffLen];
    bzero(&mac_ctx, sizeof(mac_ctx));

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

    sf_serial_mac_init((struct sf_serial_mac_ctx *) mac_ctx, ctx.tty_fd, rx, tx,
            read_evt, write_evt);

    thread get(userinput, &ctx, (struct sf_serial_mac_ctx*) &mac_ctx);
    get.detach();

    while (*(ctx.iBuff) != 'q')
    {
        if (read(ctx.tty_fd, ctx.iBuff, 1) > 0)
            write(STDOUT_FILENO, ctx.iBuff, 1); // if new data is available on the serial port, print it out
        sf_serial_mac_entry((struct sf_serial_mac_ctx *) mac_ctx);
    }

    close(ctx.tty_fd);
    delete ctx.iBuff;
    return EXIT_SUCCESS;
}
