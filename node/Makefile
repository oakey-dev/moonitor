CC=gcc
CFLAGS=-g -Wall
BINDIR=bin

daemon: $(BINDIR) daemon.o database.o collector.o
	$(CC) -lsqlite3 -pthread daemon.o database.o collector.o -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) $?

clean:
	rm -rf *.o $(BINDIR)


$(BINDIR):
	mkdir $(BINDIR)
