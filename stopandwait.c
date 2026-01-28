#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "rs232.h"

#define PORT 0
#define BAUD 9600
#define BUF 1024
#define TIMEOUT_SEC 2

int main()
{
    char role;
    char seq = '0';
    char expected_seq = '0';
    char buffer[BUF];
    char rxbuf[BUF];

    if (RS232_OpenComport(PORT, BAUD, "8N1"))
    {
        printf("Error opening RS232 port\n");
        return 1;
    }

    printf("Enter mode (t = transmitter, r = receiver): ");
    scanf(" %c", &role);
    getchar(); // clear newline

    /* ================= TRANSMITTER ================= */
    if (role == 't')
    {
        printf("STOP-AND-WAIT TRANSMITTER STARTED\n");

        while (1)
        {
            printf("Enter message: ");
            fgets(buffer, BUF - 3, stdin);   // prevent overflow

            char frame[BUF];
            snprintf(frame, sizeof(frame), "%c|%s", seq, buffer);

        resend:
            RS232_SendBuf(PORT, (unsigned char *)frame, strlen(frame));
            printf("Sent frame SEQ=%c\n", seq);

            int waited = 0;

            while (waited < TIMEOUT_SEC * 10)
            {
                int n = RS232_PollComport(PORT, (unsigned char *)rxbuf, BUF - 1);
                if (n > 0)
                {
                    rxbuf[n] = '\0';

                    if (strncmp(rxbuf, "ACK", 3) == 0 && rxbuf[3] == seq)
                    {
                        printf("ACK received for SEQ=%c\n\n", seq);
                        seq = (seq == '0') ? '1' : '0';
                        goto next_frame;
                    }
                }

                usleep(100000); // 100 ms
                waited++;
            }

            printf("Timeout! Resending...\n");
            goto resend;

        next_frame:
            ;
        }
    }

    /* ================= RECEIVER ================= */
    else if (role == 'r')
    {
        printf("STOP-AND-WAIT RECEIVER STARTED\n");

        while (1)
        {
            int n = RS232_PollComport(PORT, (unsigned char *)rxbuf, BUF - 1);
            if (n > 0)
            {
                rxbuf[n] = '\0';

                char rx_seq = rxbuf[0];

                if (rx_seq == expected_seq)
                {
                    printf("Received (SEQ=%c): %s", rx_seq, rxbuf + 2);

                    char ack[5];
                    snprintf(ack, sizeof(ack), "ACK%c", rx_seq);
                    RS232_SendBuf(PORT, (unsigned char *)ack, strlen(ack));

                    expected_seq = (expected_seq == '0') ? '1' : '0';
                }
                else
                {
                    /* Duplicate frame → resend last ACK */
                    char ack[5];
                    snprintf(ack, sizeof(ack), "ACK%c",
                             (expected_seq == '0') ? '1' : '0');
                    RS232_SendBuf(PORT, (unsigned char *)ack, strlen(ack));
                }
            }

            usleep(100000); // reduce CPU usage
        }
    }

    else
    {
        printf("Invalid mode\n");
    }

    return 0;
}



