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

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyUSB0"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP = FALSE;


int main(int argc, char **argv)
{

    struct termios tio;
    struct termios stdio;
    struct termios old_stdio;
    int tty_fd;

    unsigned char c = 'D';
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

    tty_fd = open(MODEMDEVICE, O_RDWR | O_NONBLOCK);
    if (tty_fd < 0)
    {
        perror(MODEMDEVICE);
        exit(-1);
    }
    cfsetospeed(&tio, B115200);            // 115200 baud
    cfsetispeed(&tio, B115200);            // 115200 baud

    tcsetattr(tty_fd, TCSANOW, &tio);
    while (c != 'q')
    {
        if (read(tty_fd, &c, 1) > 0)
            write(STDOUT_FILENO, &c, 1); // if new data is available on the serial port, print it out
        if (read(STDIN_FILENO, &c, 1) > 0)
            write(tty_fd, &c, 1); // if new data is available on the console, send it to the serial port
    }

    close(tty_fd);
    /* restore previous port settings */
    tcsetattr(STDOUT_FILENO, TCSANOW, &old_stdio);

    return EXIT_SUCCESS;
}

