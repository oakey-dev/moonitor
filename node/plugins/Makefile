CC=gcc
CFLAGS=-O2 -fPIC
SONAME=cpu_avg.so
VERSION=1.0

libs: cpu_avg.o
	gcc ${CFLAGS} -shared -Wl,-soname,${SONAME} -o lib/${SONAME}.${VERSION} cpu_avg.o

#cpu_avg.o: cpu_avg.c
#	gcc -c -g cpu_avg.c -o cpu_avg.o

clean:
	rm -f *.o lib/*.so.*

