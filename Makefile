CC=gcc
CFLAGS=-ggdb -O0
LDFLAGS=-lpthread

all: agent manager

agent: agent.c queue.c comm.c event.c
	$(CC) $^ $(CFLAGS) -o $@ $(LDFLAGS)

manager: manager.c
	$(CC) $^ $(CFLAGS) -o $@ $(LDFLAGS)
    





