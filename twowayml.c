#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "rs232.h"

#define BUF_SIZE 4096

int cport_nr = 0;
int running = 1;

/* -------- Receiver Thread -------- */
void* receive_thread(void* arg)
{
    unsigned char rxbuf[BUF_SIZE];
    int n;

    while (running)
    {
        n = RS232_PollComport(cport_nr, rxbuf, BUF_SIZE - 1);

        if (n > 0)
        {
            rxbuf[n] = 0;  // Null-terminate
            printf("\nRX: %s\n", rxbuf);
            fflush(stdout);
            printf("You: ");  // prompt after receiving
            fflush(stdout);
        }

        usleep(1000);  // small sleep to avoid CPU hogging
    }
    return NULL;
}

/* -------- Sender Thread -------- */
void* send_thread(void* arg)
{
    char txbuf[BUF_SIZE];
    int pos;
    int c;

    printf("Type your message. Press Enter to send.\n");
    printf("Use Ctrl+N to insert a newline in your message.\n");

    while (running)
    {
        pos = 0;
        printf("You: ");
        fflush(stdout);

        while ((c = getchar()) != EOF)
        {
            if (c == '\n') // Enter key -> send message
            {
                if (pos > 0)
                {
                    txbuf[pos] = '\0';
                    RS232_SendBuf(cport_nr, (unsigned char*)txbuf, pos);
                    printf("TX: %s\n", txbuf);
                    fflush(stdout);
                }
                break;
            }
            else if (c == 14) // Ctrl+N = newline inside message
            {
                txbuf[pos++] = '\n';
                putchar('\n');
                fflush(stdout);
            }
            else
            {
                txbuf[pos++] = c;
                putchar(c);  // echo
                fflush(stdout);
            }

            if (pos >= sizeof(txbuf) - 1) // prevent overflow
                break;
        }
    }
    return NULL;
}

int main()
{
    int bdrate = 9600;
    char mode[] = {'8','N','1',0};

    pthread_t rx_thread, tx_thread;

    if (RS232_OpenComport(cport_nr, bdrate, mode))
    {
        printf("Cannot open serial port\n");
        return 1;
    }

    printf("Multithreaded serial chat started\n");

    pthread_create(&rx_thread, NULL, receive_thread, NULL);
    pthread_create(&tx_thread, NULL, send_thread, NULL);

    pthread_join(rx_thread, NULL);
    pthread_join(tx_thread, NULL);

    RS232_CloseComport(cport_nr);
    return 0;
}