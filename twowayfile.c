#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#include <fcntl.h>
#endif

#include "rs232.h"

#define BUF_SIZE   4096
#define FILE_CHUNK 1024

#ifndef _WIN32
void make_stdin_nonblocking(void)
{
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
}
#endif

/* ================= SEND FILE ================= */

void send_file(int port, const char *filename)
{
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        printf("Cannot open file: %s\n", filename);
        return;
    }

    fseek(fp, 0, SEEK_END);
    long filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char header[512];
    snprintf(header, sizeof(header),
             "FILE %s %ld\n", filename, filesize);

    RS232_SendBuf(port, (unsigned char *)header, strlen(header));

    unsigned char buf[FILE_CHUNK];
    size_t n;

    while ((n = fread(buf, 1, FILE_CHUNK, fp)) > 0) {
        RS232_SendBuf(port, buf, n);
    }

    fclose(fp);
    printf("File sent successfully: %s (%ld bytes)\n",
           filename, filesize);
}

/* ================= MAIN ================= */

int main(void)
{
    int cport_nr = 0;      /* COM1 or /dev/ttyS0 */
    int bdrate   = 9600;
    char mode[]  = {'8','N','1',0};

    unsigned char rxbuf[BUF_SIZE];
    char txbuf[512];

    /* Receive state */
    static int receiving = 0;
    static long bytes_left = 0;
    static FILE *recv_fp = NULL;
    static char header_buf[512];
    static int header_len = 0;

#ifndef _WIN32
    make_stdin_nonblocking();
#endif

    if (RS232_OpenComport(cport_nr, bdrate, mode)) {
        printf("Cannot open serial port\n");
        return 1;
    }

    printf("=====================================\n");
    printf(" Two-Way Serial File Transfer Ready\n");
    printf(" Command: send <filename>\n");
    printf("=====================================\n\n");

    while (1)
    {
        /* ------------ RECEIVE ------------ */
        int n = RS232_PollComport(cport_nr, rxbuf, BUF_SIZE);

        if (n > 0)
        {
            int i = 0;

            while (i < n)
            {
                /* ---------- HEADER MODE ---------- */
                if (!receiving)
                {
                    char c = rxbuf[i++];
                    header_buf[header_len++] = c;

                    if (c == '\n')
                    {
                        header_buf[header_len] = 0;
                        header_len = 0;

                        if (!strncmp(header_buf, "FILE ", 5))
                        {
                            char filename[256];
                            sscanf(header_buf,
                                   "FILE %255s %ld",
                                   filename, &bytes_left);

                            recv_fp = fopen(filename, "wb");
                            receiving = 1;

                            printf("Receiving file: %s (%ld bytes)\n",
                                   filename, bytes_left);
                        }
                        else
                        {
                            printf("%s", header_buf);
                        }
                    }
                }
                /* ---------- DATA MODE ---------- */
                else
                {
                    int chunk = n - i;
                    if (chunk > bytes_left)
                        chunk = bytes_left;

                    fwrite(&rxbuf[i], 1, chunk, recv_fp);
                    i += chunk;
                    bytes_left -= chunk;

                    if (bytes_left == 0)
                    {
                        fclose(recv_fp);
                        recv_fp = NULL;
                        receiving = 0;
                        printf("File received successfully ✅\n");
                    }
                }
            }
        }

        /* ------------ SEND ------------ */
        if (fgets(txbuf, sizeof(txbuf), stdin))
        {
            if (!strncmp(txbuf, "send ", 5))
            {
                char *fname = txbuf + 5;
                fname[strcspn(fname, "\r\n")] = 0;
                send_file(cport_nr, fname);
            }
            else
            {
                RS232_SendBuf(cport_nr,
                              (unsigned char *)txbuf,
                              strlen(txbuf));
            }
        }

#ifdef _WIN32
        Sleep(20);
#else
        usleep(20000);
#endif
    }

    return 0;
}