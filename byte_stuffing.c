#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "rs232.h"

#define PORT 0
#define BAUD 9600
#define BUF 1024
#define TIMEOUT_SEC 2

#define FLAG 0x7E
#define ESC  0x7D

/* ================= BYTE STUFFING ================= */

int byte_stuff(unsigned char *input, int len, unsigned char *output)
{
    int j = 0;
    for (int i = 0; i < len; i++)
    {
        if (input[i] == FLAG || input[i] == ESC)
        {
            output[j++] = ESC;
            output[j++] = input[i] ^ 0x20;
        }
        else
        {
            output[j++] = input[i];
        }
    }
    return j;
}

int byte_destuff(unsigned char *input, int len, unsigned char *output)
{
    int j = 0;
    for (int i = 0; i < len; i++)
    {
        if (input[i] == ESC)
        {
            i++;
            output[j++] = input[i] ^ 0x20;
        }
        else
        {
            output[j++] = input[i];
        }
    }
    return j;
}

/* ================= MAIN ================= */

int main()
{
    char role;
    char seq = 0;
    char expected_seq = 0;

    unsigned char txbuf[BUF];
    unsigned char rxbuf[BUF];

    if (RS232_OpenComport(PORT, BAUD, "8N1"))
    {
        printf("Error opening RS232 port\n");
        return 1;
    }

    printf("Enter mode (t = transmitter, r = receiver): ");
    scanf(" %c", &role);
    getchar();

    /* ================= TRANSMITTER ================= */
    if (role == 't')
    {
        printf("STOP-AND-WAIT TRANSMITTER (Byte Stuffing)\n");

        while (1)
        {
            char message[BUF];
            printf("Enter message: ");
            fgets(message, BUF - 10, stdin);

            unsigned char frame[BUF];
            unsigned char stuffed[BUF];

            int index = 0;

            frame[index++] = seq;
            int msg_len = strlen(message);

            memcpy(&frame[index], message, msg_len);
            index += msg_len;

            int stuffed_len = byte_stuff(frame, index, stuffed);

        resend:

            unsigned char final_frame[BUF];
            int k = 0;

            final_frame[k++] = FLAG;
            memcpy(&final_frame[k], stuffed, stuffed_len);
            k += stuffed_len;
            final_frame[k++] = FLAG;

            RS232_SendBuf(PORT, final_frame, k);

            printf("Sent Frame SEQ=%d\n", seq);

            int waited = 0;

            while (waited < TIMEOUT_SEC * 10)
            {
                int n = RS232_PollComport(PORT, rxbuf, BUF);
                if (n > 0)
                {
                    if (rxbuf[0] == 'A' && rxbuf[1] == 'C' && rxbuf[2] == 'K' && rxbuf[3] == seq)
                    {
                        printf("ACK received for SEQ=%d\n\n", seq);
                        seq = (seq == 0) ? 1 : 0;
                        goto next;
                    }
                }

                usleep(100000);
                waited++;
            }

            printf("Timeout! Resending...\n");
            goto resend;

        next:
            ;
        }
    }

    /* ================= RECEIVER ================= */
    else if (role == 'r')
    {
        printf("STOP-AND-WAIT RECEIVER (Byte Stuffing)\n");

        while (1)
        {
            int n = RS232_PollComport(PORT, rxbuf, BUF);

            if (n > 0)
            {
                int start = -1, end = -1;

                for (int i = 0; i < n; i++)
                {
                    if (rxbuf[i] == FLAG)
                    {
                        if (start == -1)
                            start = i;
                        else
                        {
                            end = i;
                            break;
                        }
                    }
                }

                if (start != -1 && end != -1)
                {
                    unsigned char destuffed[BUF];
                    int len = byte_destuff(&rxbuf[start + 1], end - start - 1, destuffed);

                    char rx_seq = destuffed[0];

                    if (rx_seq == expected_seq)
                    {
                        printf("Received (SEQ=%d): ", rx_seq);
                        fwrite(&destuffed[1], 1, len - 1, stdout);

                        unsigned char ack[4] = {'A','C','K',rx_seq};
                        RS232_SendBuf(PORT, ack, 4);

                        expected_seq = (expected_seq == 0) ? 1 : 0;
                    }
                    else
                    {
                        unsigned char ack[4] = {'A','C','K',
                                                (expected_seq == 0) ? 1 : 0};
                        RS232_SendBuf(PORT, ack, 4);
                    }
                }
            }

            usleep(100000);
        }
    }

    else
    {
        printf("Invalid mode\n");
    }

    return 0;
}
