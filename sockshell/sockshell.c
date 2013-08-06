/* 
   A rudimentary command line interface
   receiving input and sending output on a TCP Socket
 */

#include "sockshell.h"

/* It is ESSENTIAL to bind to localhost as no authentication and encryption is implemented yet */
#define ADDRESS "127.0.0.1"
#define ADDRESS6 "::1"
#define PORT 60000
#define BUFSIZE 128
#define BACKLOG 1024 /* maximum queue length for the listening socket */

char* ps = "moonitor>"; /* prompt string */

void* start_sockshell(void* p_arg){
	assert(sizeof(void*) >= sizeof(int));
	int consock;
	pthread_t last_session;
	socklen_t socksize = sizeof(struct sockaddr_in);

	struct sockaddr_in addr;
	memset(&addr,0,sizeof(addr)); /* zero out the struct */
	addr.sin_family = AF_INET;	
	addr.sin_addr.s_addr = inet_addr(ADDRESS);
	addr.sin_port = htons(PORT);

	int sock = socket(AF_INET,SOCK_STREAM,0);
	bind(sock,(struct sockaddr*)&addr,sizeof(addr));
	listen(sock,BACKLOG);

	for(;;){ /* listen for incoming connections, open a session for each one */
		consock = accept(sock,(struct sockaddr*)&addr,&socksize);
		pthread_create(&last_session,NULL,session,(void*)consock);
	}

	return 0;
}

void* session(void* p_sock){
	char buf[BUFSIZE+1];
	buf[BUFSIZE] = '\0'; /* ensure proper string termination */
	int consock = (int)p_sock;

	pthread_detach(pthread_self());

	snprintf(buf,BUFSIZE,"Welcome to moonitor shell. Session thread id: %ld\n",(long)pthread_self());
	nwrite(consock,buf,strlen(buf));

	close(consock);
	return 0;
}

/* encapsulation of write() to handle errors better */
int nwrite(int fd, const void* buf,size_t count){
	int n=0;
	size_t written=0;
	char* bytebuf=(char*)buf;
	while(written < count){
		n = write(fd,(void*)&(bytebuf[written]),count);
		if(n<0){
			perror("error writing to file descriptor");
			return n; /* propagate error status */
		} else {
			written += n;
		}
	}
	return written;
}
