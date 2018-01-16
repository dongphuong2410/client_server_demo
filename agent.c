#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "comm.h"
#include "queue.h"
#include "simul_params.h"

static void handle_signal(void);
static void create_queue(void);
static void destroy_queue(void);
static void create_threads(void);
static void destroy_threads(void);
static void parse_msg(const char *msg);

static void *recv_thread(void *data);
static void *module_thread(void *data);

int running = 1;

static struct sigaction act;
void sig_handler(int signal)
{
    running = 0;
}

/* Global queues */
queue_t *send_queue;
queue_t *recv_queue;

/* Global threads */
pthread_t recv_thread_id;
pthread_t module_thread_ids[MODULE_NO];

int main(int argc, char **argv)
{
    handle_signal();
    create_queue();
    create_threads();
    nw_connect();

    /* request connect */
    nw_write("ACQ", 3);

    int iCount = 0;
    while (running) {   //TODO: this should be check for nw_okay
        char *msg = queue_dequeue(recv_queue);
        if (msg) {
            printf("%s\n", msg);
            parse_msg(msg);
        }
        else {
            iCount++;
            if (iCount >= HANG_TEST_LOOP) {
                iCount = 0;
                usleep(10 * 1000);
            }
        }
        usleep(10 * 1000);
    }

    nw_destroy();
    destroy_threads();
    destroy_queue();
    return 0;
}

static void handle_signal(void)
{
    act.sa_handler = sig_handler;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    sigaction(SIGTERM, &act, NULL);
    sigaction(SIGINT, &act, NULL);
}

static void create_queue(void)
{
    send_queue = queue_init(QUEUE_SIZE);
    recv_queue = queue_init(QUEUE_SIZE);
}

static void destroy_queue(void)
{
    queue_destroy(send_queue);
    queue_destroy(recv_queue);
}

static void create_threads(void)
{
    if (pthread_create(&recv_thread_id, NULL, recv_thread, NULL))
    {
        printf("Error creating recv thread\n");
    }
    else {
        printf("Receive thread created\n");
    }
    int i;
    for (i = 0; i < MODULE_NO; i++) {
        if (pthread_create(&module_thread_ids[i], NULL, module_thread, NULL)) {
            printf("Error creating module thread\n");
        }
        else {
            printf("Module thread %d created\n", i);
        }
    }
}

static void destroy_threads(void)
{
    pthread_join(recv_thread_id);
    printf("Receive thread destroyed\n");
    int i;
    for (i = 0; i < MODULE_NO; i++) {
        pthread_join(module_thread_ids[i]);
        printf("Module thread %d destroyed\n", i);
    }
}

static void *recv_thread(void *data)
{
    static char buff[PACKET_LEN];
    while (running) {
        if (!nw_okay()) {
            usleep(10000);
        }
        else {
            int bytes = nw_read(buff);
            if (bytes <= 0) {
                usleep(101000);
            }
            queue_enqueue(recv_queue, buff);
        }
    }
}

static void parse_msg(const char *msg)
{
    if (strncmp(msg, "ACK", PACKET_LEN)) {
        printf("ACK received\n");
    }
}

static void *module_thread(void *data)
{
    while (running) {
        queue_enqueue(send_queue, "EVT");
        usleep(500 * 1000);
    }
}
