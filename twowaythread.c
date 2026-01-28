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
            rxbuf[n] = 0;
            printf("RX: %s", rxbuf);
            fflush(stdout);
        }

        usleep(1000);  // small sleep to avoid CPU hogging
    }
    return NULL;
}

/* -------- Sender Thread -------- */
void* send_thread(void* arg)
{
    char txbuf[512];

    while (running)
    {
        if (fgets(txbuf, sizeof(txbuf), stdin) != NULL)
        {
            RS232_cputs(cport_nr, txbuf);
            printf("TX: %s", txbuf);
            fflush(stdout);
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

    printf("Multithreaded serial communication started\n");

    pthread_create(&rx_thread, NULL, receive_thread, NULL);
    pthread_create(&tx_thread, NULL, send_thread, NULL);

    pthread_join(rx_thread, NULL);
    pthread_join(tx_thread, NULL);

    return 0;
}