#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdarg.h>

#define CMD_LEN 1024
#define PACKET_LEN 2000

static void thread_init(void);
static void *run(void *data);
static void *connection_handler(void *data);
static void mgrlog(const char *message, ...);

int quit = 0;
FILE *logfd;
pthread_t network_thread;

int main(int argc, char **argv)
{
    logfd = fopen("manager.log", "a+");

    /* Create task threads */
    thread_init();

    /* User command */
    char command[CMD_LEN];
    while (1) {
        printf("\nmgr>");
        scanf("%s", command);
        if (!strncmp(command, "quit", CMD_LEN)) {
            quit = 1;
            printf("Bye!\n");
            break;
        }
        else {
            printf("Receive : %s", command);
        }
    }

    pthread_join(network_thread, NULL);
    fclose(logfd);
    return 0;
}

static void *connection_handler(void *data)
{
    char buff[PACKET_LEN];
    int *client_socket = (int *)data;
    while (!quit) {
        int bytes = recv(*client_socket, buff, PACKET_LEN, 0);
        if (bytes > 0) {
            mgrlog("Receive : %s\n", buff);
            if (!strncmp(buff, "ACQ", PACKET_LEN)) {
                printf("\nReceive ACQ");
                printf("\nmgr>");
                send(*client_socket, "ACK", 3, 0);
                mgrlog("Send : %s\n", "ACK");
            }
        }
        else {
            mgrlog("Connection close ..\n");
            break;
        }
    }
}

static void thread_init(void)
{
    if (pthread_create(&network_thread, NULL, run, NULL))
    {
        printf("Error creating thread\n");
    }
}

static void *run(void *data)
{
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        mgrlog("ERROR could not create socket\n");
    }
    struct sockaddr_in server_addr, client_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(2424);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(server_addr.sin_zero, '\0', sizeof(server_addr.sin_zero));
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        mgrlog("Bind failed\n");
        return;
    }
    listen(server_socket, 3);
    int c = sizeof(struct sockaddr_in);
    mgrlog("Waiting for incomming connections...\n");
    while (!quit) {
        int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, (socklen_t*)&c);
        if (client_socket < 0) {
            mgrlog("Accept failed\n");
        }
        mgrlog("Connection accepted\n");
        if (pthread_create(&network_thread, NULL, connection_handler, &client_socket))
        {
            mgrlog("Error creating thread\n");
        }
    }
}

static void mgrlog(const char *message, ...)
{
    va_list args;
    va_start(args, message);
    vfprintf(logfd, message, args);
    fflush(logfd);
    va_end(args);
}
