#include <stdio.h>
#include <stdlib.h>

#ifndef _WIN32
#include <unistd.h>
#endif

#include "rs232.h"

int main()
{
    int cport_nr = 0;
    int bdrate = 9600;
    unsigned char buf[4096];

    char mode[] = {'8','N','1',0};

    if (RS232_OpenComport(cport_nr, bdrate, mode)) {
        fprintf(stderr, "Cannot open serial port\n");
        return 1;
    }

    printf("Optimized RX running...\n");

    while (1) {
        int n = RS232_PollComport(cport_nr, buf, sizeof(buf)-1);

        if (n > 0) {
            buf[n] = 0;  // null terminate
            fwrite(buf, 1, n, stdout);
            fflush(stdout);
        }

#ifndef _WIN32
        usleep(2000);  // 2ms sleep for fast polling
#else
        Sleep(2);
#endif
    }

    return 0;
}