#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#endif

#include "rs232.h"

#define BUF_SIZE 4096

#ifndef _WIN32
void make_stdin_nonblocking()
{
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
}
#endif

int main()
{
    int cport_nr = 0;     /* COM1 or /dev/ttyS0 */
    int bdrate   = 9600;  /* 9600 baud */
    char mode[]  = {'8','N','1',0};

    unsigned char rxbuf[BUF_SIZE];
    char txbuf[512];

    int n;

#ifndef _WIN32
    make_stdin_nonblocking();   // So fgets doesn't block
#endif

    if (RS232_OpenComport(cport_nr, bdrate, mode)) {
        printf("Cannot open serial port!\n");
        return 1;
    }

    printf("Two-way serial communication ready.\n");
    printf("Type a line and press ENTER to send.\n\n");

    while (1)
    {
        /* ------------ RECEIVE (full strings from serial) ------------ */
        n = RS232_PollComport(cport_nr, rxbuf, BUF_SIZE - 1);

        if (n > 0) {
            rxbuf[n] = 0;

            // clean control characters
            for (int i = 0; i < n; i++)
                if (rxbuf[i] < 32 && rxbuf[i] != '\n' && rxbuf[i] != '\r')
                    rxbuf[i] = '.';

            printf("%s", rxbuf);
            fflush(stdout);
        }

        /* ------------ SEND (full strings typed by user) ------------ */
        if (fgets(txbuf, sizeof(txbuf), stdin) != NULL) {
            RS232_cputs(cport_nr, txbuf);
            printf("Chintan send : %s", txbuf);
            fflush(stdout);
        }

#ifdef _WIN32
        // Sleep(50000000000);
#else
        usleep(50);  // 50 ms
#endif
    }

    return 0;
}
