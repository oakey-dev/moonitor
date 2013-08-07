CC=gcc
CFLAGS=-g -Wall
BINDIR=bin

$(BINDIR)/daemon: $(BINDIR) daemon.o database.o collector.o listener.o
	$(CC) -lsqlite3 -lpthread daemon.o database.o collector.o listener.o -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) $?

clean:
	rm -rf *.o $(BINDIR)

$(BINDIR):
	mkdir $(BINDIR)
