CC=gcc
CFLAGS=-c -g
BINDIR=bin

daemon: daemon.o collector.o
	$(CC) -pthread daemon.o collector.o -o bin/daemon

daemon.o: daemon.c
	$(CC) $(CFLAGS) daemon.c

collector.o: collector.c
	$(CC) $(CFLAGS) collector.c

clean:
	rm -rf *.o $(BINDIR)

$(BINDIR):
	mkdir $(BINDIR)
