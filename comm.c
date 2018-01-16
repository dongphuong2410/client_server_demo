#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "comm.h"

int sock;
static int network_status = NW_STATUS_DISCONNECTED;

int nw_connect()
{
    struct sockaddr_in server;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        printf("Could not create scoket\n");
    }

    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(2424);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        printf("Connect failed\n");
        return 1;
    }
    printf("Connected\n");
    network_status = NW_STATUS_OK;
    return 0;
}

void nw_destroy()
{
    printf("Destroy network\n");
    close(sock);
}

int nw_okay()
{
    return (network_status == NW_STATUS_OK);
}

int nw_write(const char *buff, size_t length)
{
    int res = send(sock, buff, length, 0);
    if (res < 0)
        printf("Send data error\n");
    return res;
}

int nw_read(char *buff)
{
    return recv(sock, buff, PACKET_LEN, 0);
}


