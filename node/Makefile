CC=gcc
CFLAGS=-c -g
BINDIR=bin

daemon: $(BINDIR) daemon.o database.o collector.o
	$(CC) -lsqlite3 -pthread daemon.o database.o collector.o -o bin/daemon

daemon.o: daemon.c
	$(CC) $(CFLAGS) daemon.c

collector.o: collector.c
	$(CC) $(CFLAGS) collector.c

database.o: database.c
	$(CC) $(CFLAGS)  database.c

clean:
	rm -rf *.o $(BINDIR)

$(BINDIR):
	mkdir $(BINDIR)
