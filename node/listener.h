#ifndef _LISTENER_H_
#define _LISTENER_H_

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
#include <signal.h>
#include <sqlite3.h>

#define pthread_error(error_number,message)\
	do{ errno =  error_number; perror(message); } while(0)

void* listener_thread(void*);
void* session(void*);
int nwrite(int,const void*,size_t);
void catch_sigpipe(int);

extern sqlite3* sqlite_db;
int db_query(char* query,int* p_sock);
int callback(void* p_sock, int column_count, char** column_text, char** column_names);

#endif /* _LISTENER_H_ */
