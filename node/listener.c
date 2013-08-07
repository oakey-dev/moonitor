/* 
   A rudimentary command line interface
   receiving input and sending output on a TCP Socket
 */

#include "listener.h"

/* It is ESSENTIAL to bind to localhost as no authentication and encryption is implemented yet */
#define DEFAULT_HOST "127.0.0.1"
#define DEFAULT_PORT 2130
#define BUFSIZE 128
#define BACKLOG 1024 /* maximum queue length for the listening socket */

char* ps = "moonitor>"; /* prompt string */

void* listener_thread(void* p_arg){
	assert(sizeof(void*) >= sizeof(int));
	int consock;
	pthread_t last_session;
	socklen_t socksize = sizeof(struct sockaddr_in);

	/* make sure SIGPIPE doesn't result in termination */
	struct sigaction sig_act;
	memset(&sig_act,0,sizeof(struct sigaction));
	sig_act.sa_handler = catch_sigpipe;
	sigaction(SIGPIPE,&sig_act,NULL);

	struct sockaddr_in addr;
	memset(&addr,0,sizeof(addr)); /* zero out the struct */
	addr.sin_family = AF_INET;	
	addr.sin_addr.s_addr = inet_addr(DEFAULT_HOST);
	addr.sin_port = htons(DEFAULT_PORT);

	int sock = socket(AF_INET,SOCK_STREAM,0);
	bind(sock,(struct sockaddr*)&addr,sizeof(addr));
	listen(sock,BACKLOG);

	int status;
	for(;;){ /* listen for incoming connections, open a session for each one */
		consock = accept(sock,(struct sockaddr*)&addr,&socksize);
		if(consock<0){
			perror("error accepting connections on socket");
		}
#ifndef NDEBUG
		printf("sockshell main thread (thread id:%ld) accepted connection\n",(long)pthread_self());
#endif
		status = pthread_create(&last_session,NULL,session,(void*)consock);
		if(status != 0){
			pthread_error(status,"error creating thread for sockshell session");
		}
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

	int status=0;
	while(status>=0){
		status = nwrite(consock,ps,strlen(ps));
		read(consock,buf,BUFSIZE);
	}

	if(close(consock) != 0){
		perror("error closing socket");
	}
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

void catch_sigpipe(int i){
	printf("caught SIGPIPE\n");
}
