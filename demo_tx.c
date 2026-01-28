
/**************************************************

file: demo_tx.c
purpose: simple demo that transmits characters to
the serial port and print them on the screen,
exit the program by pressing Ctrl-C

compile with the command: gcc demo_tx.c rs232.c -Wall -Wextra -o2 -o test_tx

**************************************************/

// #include <stdlib.h>
// #include <stdio.h>

// #ifdef _WIN32
// #include <Windows.h>
// #else
// #include <unistd.h>
// #endif

// #include "rs232.h"



// int main()
// {
//   int i=0,
//       cport_nr=0,        /* /dev/ttyS0 (COM1 on windows) */
//       bdrate=9600;       /* 9600 baud */

//   char mode[]={'8','N','1',0},
//        str[2][512];


//   strcpy(str[0], "The quick brown fox jumped over the lazy grey dog.\n");

//   strcpy(str[1], "Happy serial programming!\n");


//   if(RS232_OpenComport(cport_nr, bdrate, mode))
//   {
//     printf("Can not open comport\n");

//     return(0);
//   }

//   while(1)
//   {
//     RS232_cputs(cport_nr, str[i]);

//     printf("sent: %s\n", str[i]);

// #ifdef _WIN32
//     Sleep(1000);
// #else
//     usleep(1000000);  /* sleep for 1 Second */
// #endif

//     i++;

//     i %= 2;
//   }

//   return(0);
// }

// #include <stdlib.h>
// #include <stdio.h>

// #ifdef _WIN32
// #include <Windows.h>
// #else
// #include <unistd.h>
// #endif

// #include "rs232.h"

// int main()
// {
//     int cport_nr = 0;     /* /dev/ttyS0 */
//     int bdrate   = 9600;  /* 9600 baud */
//     char mode[]  = {'8','N','1',0};
//     int ch;

//     printf("Opening serial port...\n");

//     if (RS232_OpenComport(cport_nr, bdrate, mode)) {
//         printf("Cannot open comport\n");
//         return 0;
//     }

//     printf("Port open. Type characters to send.\n");
//     printf("Press Ctrl-C to exit.\n");

//     while (1) {
//         ch = getchar();               // Wait for user to type a character

//         RS232_SendByte(cport_nr, ch); // Send that character
//         printf("Sent: %c\n", ch);
//     }

//     return 0;
// }




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

    char mode[] = {'8','N','1',0};

    const char *msg1 = "The quick brown fox jumped over the lazy grey dog.\n";
    const char *msg2 = "Happy serial programming!\n";

    printf("Optimized TX running...\n");

    if (RS232_OpenComport(cport_nr, bdrate, mode)) {
        fprintf(stderr, "Cannot open serial port\n");
        return 1;
    }

    int toggle = 0;

    while (1) {
        if (toggle == 0)
            RS232_cputs(cport_nr, msg1);
        else
            RS232_cputs(cport_nr, msg2);

        toggle ^= 1;

#ifndef _WIN32
        usleep(200000);  // send every 200ms instead of 1sec
#else
        Sleep(200);
#endif
    }

    return 0;
}

