#ifndef SOCKSHELLH
#define SOCKSHELLH

#include <assert.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <stdio.h>

void* start_sockshell(void*);
void* session(void*);
int nwrite(int,const void*,size_t);

#endif /* SOCKSHELLH */
