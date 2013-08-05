#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <sqlite3.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <fcntl.h>           /* For O_* constants */
#include <semaphore.h>

#include "collector.h"

int munin_cpu() {
   printf("cpu");
}
int munin_load() {
   printf("load");
}
int munin_memory() {
   printf("memory");
}


void* collector_thread(void* arg) {
   //struct setting settings = (struct setting)arg;
   struct queue *que, *tmp;
   pid_t pid, cpid;
   int i;
   char str[sizeof(void*)*2+2];
   
   // init queue
   que = malloc(sizeof(struct queue));
   if ( que == NULL ) {
      debug("!! not enough memory");
      exit(1);
   }
   que->timestamp = 0;
   que->plugins = malloc(4*sizeof(struct plugin*));
   que->next = NULL;

   // init startup plugins TODO read from file
   struct plugin* p;
   i = 0;

      p = malloc(sizeof(struct plugin));
      p->interval = 60;
      p->cmd = &munin_load; 
      que->plugins[i] = p;
      i++;

      p = malloc(sizeof(struct plugin));
      p->interval = 30;
      p->cmd = &munin_memory; 
      que->plugins[i] = p;
      i++;

      p = malloc(sizeof(struct plugin));
      p->interval = 10;
      p->cmd = &munin_cpu; 
      que->plugins[i] = p;
      i++;

      que->plugins[i] = NULL;

   // validate / init plugins
   for ( i=0; (p = que->plugins[i]) != NULL; i++ ) {
      // set minimal interval 
      if ( p->interval < 5 ) {
         p->interval = 5;
      }
      snprintf(str, (sizeof(void*)*2+10), "/moonit%p", p->cmd);
      // overwrite 0x infront of the pointer
      *(str+7) = 'o';
      *(str+8) = 'r';
      p->lock = sem_open(str,O_CREAT); 
   }
   
   // loop the process
   while ( 1 ) {
      if ( que->timestamp <= time(NULL) ) { 
         for ( i=0; (p = que->plugins[i]) != NULL; i++ ) {
            // try locking 
            if ( sem_trywait(p->lock) == 0 ) {
               // fork plugin as daemon
               if ( (pid = fork()) == 0 ) {
                  if ( (cpid = fork()) == 0 ) {
                     // run plugin
                     int ret = (p->cmd)();
                     sem_post(p->lock);
                     if ( ret != 0 ) {
                        fprintf(stderr, "Error: Command %x returned error code %d\n", p->cmd, ret);
                     }
                  }
                  else if ( pid < 0 ) { // error forking
                     fprintf(stderr, "Error: could not fork daemon %x", p->cmd);
                     sem_post(p->lock);
                     exit(1);
                  }
                  exit(0); // don't wait!
               }
               else if ( pid < 0 ) { // error forking
                  fprintf(stderr, "Error: could not fork child %x", p->cmd);
                  sem_post(p->lock);
               }
               else {
                  waitpid(pid, NULL, WCONTINUED);
               }
               // re-add plugin
               queue_add(que, p, (time_t)(time(NULL)+p->interval));
            }
            // throw error if service is still running and re-add in one sec.
            else {
               fprintf(stderr, "Error: runtime for %x is to short (%d)\n", p->cmd, p->interval);
               queue_add(que, p, (time_t)(time(NULL)+1));
            }
         }
         // delete current que entry and use ->next
         tmp = que->next;
         free(que->plugins);
         free(que);
         que = tmp;
      }
      sleep(1);
   }
}

struct queue* queue_add(struct queue* que, struct plugin* p, time_t time) {
   if ( que == NULL || que->timestamp > time ) {
      struct queue* next = NULL;
      if ( que != NULL ) {
         next = que;
      }
      que = malloc(sizeof(struct queue));
      if ( que == NULL ) {
         fprintf(stderr, "Critical: not enoth free Memory\n");
         exit(1);
      }
      que->timestamp = time;
      que->plugins = malloc(2*sizeof(struct plugin*));
      if ( que->plugins == NULL ) {
         fprintf(stderr, "Critical: not enoth free Memory\n");
         exit(1);
      }
      que->plugins[0] = p;
      que->plugins[1] = NULL;
      que->next = next;
   }
   else if ( que->timestamp < time ) {
      que->next = queue_add(que->next, p, time);
   }
   else if ( que->timestamp == time ) {
      int c = 0;
      while ( que->plugins[c] != NULL ) c++;
      que->plugins = realloc(que->plugins, (c+1)*sizeof(struct plugins*));
      if ( que->plugins == NULL ) {
         fprintf(stderr, "Critical: not enoth free Memory\n");
         exit(1);
      }
      que->plugins[c] = p;
      que->plugins[c+1] = NULL;
   }
   else {
      fprintf(stderr, "Critical: internal logic error\n");
      exit(255);
   }
   return que;
}

