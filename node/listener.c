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
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

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
		pthread_mutex_lock(&lock);
		consock = accept(sock,(struct sockaddr*)&addr,&socksize);
		if(consock<0){
			perror("error accepting connections on socket");
		}
#ifndef NDEBUG
		printf("sockshell main thread (thread id:%ld) accepted connection\n",(long)pthread_self());
#endif
		status = pthread_create(&last_session,NULL,session,&consock);
		if(status != 0){
			pthread_error(status,"error creating thread for sockshell session");
			pthread_mutex_unlock(&lock); 
			/* ensure that program continues to run
			 * even if the new thread fails to run
			 */
		}
	}

	return 0;
}

void* session(void* p_sock){
	char buf[BUFSIZE+1];
	buf[BUFSIZE] = '\0'; /* ensure proper string termination */
	int consock = *(int*)p_sock;
	pthread_mutex_unlock(&lock); /* allow creation of next thread */

	pthread_detach(pthread_self());

	snprintf(buf,BUFSIZE,"Welcome to moonitor shell. Session thread id: %ld\n",(long)pthread_self());
	nwrite(consock,buf,strlen(buf));

	int status=0;
	while(status>=0){
		status = nwrite(consock,ps,strlen(ps));
		read(consock,buf,BUFSIZE);
		/* TODO implement commands
		 * - help [command]
		 * - plugins: list all plugins (SELECT id FROM plugin)
		 * - config ${plugin}: (SELECT * FROM config WHERE id = ${plugin})
		 * - select ${plugin} [$from_date] [$to_date]: (SELECT * FROM ${plugin} [WHERE timestamp >= ${from_date} [AND timestamp <= ${to_date}]])
		 */
		if(strncmp(buf,"plugins",strlen("plugins")) == 0){
			db_query("SELECT id FROM plugin",&consock);
		}
		else if(buf[0] == 0x04 /* EOT, ^D */
				|| strncmp(buf,"exit",strlen("exit"))
				|| strncmp(buf,"quit",strlen("quit"))
		       )
		{
			nwrite(consock,"bye\n",5);
			break;
		}
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

int callback(void* p_sock, int column_count, char** column_text, char** column_names){
	int sock = *(int*)p_sock;
	int i;
	for(i=0;i<column_count;i++){
		nwrite(sock,column_names[i],strlen(column_names[i]));
		nwrite(sock,"=",strlen("="));
		nwrite(sock,column_text[i],strlen(column_text[i]));
		nwrite(sock,"\n",strlen("\n"));
	}
	return 0;
}

int db_query(char* query,int* p_sock){
	char* errMsg = NULL;
	if ( sqlite3_exec(sqlite_db, query, callback, p_sock, &errMsg) != SQLITE_OK ) {
		fprintf(stderr, "SQL error: %s\n", errMsg);
		sqlite3_free(errMsg);
		return 1;
	}
	return 0;
}
