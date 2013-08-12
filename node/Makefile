CC=gcc
CFLAGS=-g -Wall
BINDIR=bin

all: $(BINDIR)/daemon
	cd bin/ && ./daemon

$(BINDIR)/daemon: $(BINDIR) daemon.o database.o collector.o listener.o
	$(CC) -ldl -lsqlite3 -lpthread daemon.o database.o collector.o listener.o -o $@ 

%.o: %.c
	$(CC) -c $(CFLAGS) $?

clean:
	rm -rf *.o $(BINDIR)

$(BINDIR):
	mkdir $(BINDIR)
	ln -s ../plugins/lib $(BINDIR)/lib

.PHONY: plugins
plugins: 
	$(MAKE) -C plugins
