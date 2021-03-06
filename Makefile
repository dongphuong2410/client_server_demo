CC=gcc
#CFLAGS=-ggdb -O0 -p
CFLAGS=-O2
LDFLAGS=-lpthread

all: agent manager

agent: agent.c queue.c comm.c event.c simul_params.h
	$(CC) $^ $(CFLAGS) -o $@ $(LDFLAGS)

manager: manager.c
	$(CC) $^ $(CFLAGS) -o $@ $(LDFLAGS)
    
clean:
	rm -rf agent
	rm -rf manager





