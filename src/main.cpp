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

volatile int STOP = FALSE;

typedef struct serial_ctx
{
    int tty_fd;
    size_t buffLen;
    uint8_t *buff;
} serial_ctx_t;

ssize_t rx(void *serial_ctx, uint8_t *frameBuffer, size_t frameBufferLength);
ssize_t tx(void *serial_ctx, uint8_t *frameBuffer, size_t frameBufferLength);
void read_evt(uint8_t *frameBuffer, size_t frameBufferLength);
void write_evt(uint8_t *frameBuffer, size_t frameBufferLength);

ssize_t rx(void *serial_ctx, uint8_t *frameBuffer, size_t frameBufferLength)
{
    return 0;
}

ssize_t tx(void *serial_ctx, uint8_t *frameBuffer, size_t frameBufferLength)
{
    serial_ctx_t *ctx;
    if (serial_ctx)
    {
        ctx = (serial_ctx_t*) serial_ctx;
        if (ctx->buff == frameBuffer && ctx->buffLen == frameBufferLength)
        {
            return write(ctx->tty_fd, ctx->buff, ctx->buffLen);
        }
    }
    return -1;
}

void read_evt(uint8_t *frameBuffer, size_t frameBufferLength)
{

}
void write_evt(uint8_t *frameBuffer, size_t frameBufferLength)
{

}

int main(int argc, char **argv)
{

    SF_SERIAL_MAC_CTX mac_ctx;
    SF_SERIAL_MAC_CTX *mac_instance;
    SF_SERIAL_MAC_HAL_RX_FUNC macHalRx = rx;
    SF_SERIAL_MAC_HAL_TX_FUNC macHalTx = tx;
    SF_SERIAL_MAC_READ_EVT macRead = read_evt;
    SF_SERIAL_MAC_WRITE_EVT macWrite = write_evt;
    struct termios tio;
    struct termios stdio;
    struct termios old_stdio;
//    int tty_fd;
    serial_ctx_t ctx;
    ctx.buffLen = 1;
    ctx.buff = new uint8_t[ctx.buffLen];

//    unsigned char c = 'D';
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

    mac_instance = sf_serial_mac_init(&mac_ctx, (void*) &ctx, macHalRx,
            macHalTx, macRead, macWrite);

    ctx.tty_fd = open(MODEMDEVICE, O_RDWR | O_NONBLOCK);
    if (ctx.tty_fd < 0)
    {
        perror(MODEMDEVICE);
        exit(-1);
    }
    cfsetospeed(&tio, B115200);            // 115200 baud
    cfsetispeed(&tio, B115200);            // 115200 baud

    tcsetattr(ctx.tty_fd, TCSANOW, &tio);
    while (*(ctx.buff) != 'q')
    {
        if (read(ctx.tty_fd, ctx.buff, 1) > 0)
            write(STDOUT_FILENO, ctx.buff, 1); // if new data is available on the serial port, print it out
        if (read(STDIN_FILENO, ctx.buff, 1) > 0)
            sf_serial_mac_enqueFrame(&mac_ctx, ctx.buff, ctx.buffLen);
        //            write(ctx.tty_fd, ctx.buff, 1); // if new data is available on the console, send it to the serial port
        sf_serial_mac_entry(&mac_ctx);
    }

    close(ctx.tty_fd);
    /* restore previous port settings */
    tcsetattr(STDOUT_FILENO, TCSANOW, &old_stdio);

    return EXIT_SUCCESS;
}

