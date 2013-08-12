#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <sqlite3.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <fcntl.h>           /* For O_* constants */
#include <semaphore.h>
#include <dlfcn.h>
#include <dirent.h>
#include <string.h>

#include "collector.h"
#include "database.h"

struct plugin **plugins = NULL;

void* collector_thread(void* arg) {
   //struct setting settings = (struct setting)arg;
   struct queue *que, *tmp;
   struct plugin* p;
   pid_t pid, cpid;
   int i;
   
   // init queue
   que = malloc(sizeof(struct queue));
   if ( que == NULL ) {
      fprintf(stderr, "Critical: not enough memory!");
      exit(1);
   }
   que->timestamp = 0;
   que->plugins = NULL;
   que->next = NULL;

   load_plugins(que);

   // loop the process
   while ( 1 ) {
      if ( que->timestamp <= time(NULL) ) { 
         for ( i=0; (p = que->plugins[i]) != NULL; i++ ) {
            printf("running cmd %p, with interval %d\n", p->cmd, p->interval);
            // try locking 
            if ( sem_trywait(p->lock) == 0 ) {
               // fork plugin as daemon
               if ( (pid = fork()) == 0 ) {
                  if ( (cpid = fork()) == 0 ) {
                     // run plugin
                     char* ret = (p->cmd)();
                     sem_post(p->lock);
                     if ( ret == NULL ) {
                        fprintf(stderr, "Error: Command %p failed\n", p->cmd);
                     }
                     else {
                        sql_exec(ret);
                     }
                     free(ret);
                  }
                  else if ( pid < 0 ) { // error forking
                     fprintf(stderr, "Error: could not fork daemon %p", p->cmd);
                     sem_post(p->lock);
                     exit(1);
                  }
                  exit(0); // don't wait!
               }
               else if ( pid < 0 ) { // error forking
                  fprintf(stderr, "Error: could not fork child %p", p->cmd);
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
               fprintf(stderr, "Error: runtime for %p is to short (%d)\n", p->cmd, p->interval);
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
} // end collector_thread

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

void load_plugins(struct queue* que) {
   DIR *dir;
   struct dirent *file;
   int i = 0;

   if ( (dir = opendir("lib/")) == NULL ) {
      fprintf(stderr, "Error: can't open lib-dir");
      exit(1);
   }

   while ( (file = readdir(dir)) != NULL ) {
      if ( strstr(file->d_name, ".so") != NULL ) {
         i++;
      }
   }
   i++; // NULL at the end is needed
   que->plugins = malloc(i*sizeof(struct plugin*));
   if ( que->plugins == NULL ) {
      fprintf(stderr, "Critical: not enough memory!");
      exit(1);
   }
   plugins = malloc(i*sizeof(struct plugin*)); // extra plugins list for management (dlclose, semclose)
   if ( plugins == NULL ) {
      fprintf(stderr, "Critical: not enough memory!");
      exit(1);
   }
   printf("Number loaded plugins: %d\n", i-1);

   if ( (dir = opendir("lib/")) == NULL ) {
      fprintf(stderr, "Error: can't open lib-dir");
      exit(1);
   }

   // TODO: while file lib/*.so.*
   char *error, *path;                  // lib-error char
   int (*default_interval)();    // int function pointer  
   i = 0;
   while ( (file = readdir(dir)) != NULL ) {
      if ( strstr(file->d_name, ".so") == NULL ) {
         continue;
      }
      
      // new Plugin-Slot
      struct plugin *p = malloc(sizeof(struct plugin));

      // open .so file
      path = malloc(strlen(file->d_name)+5); // "lib/" + '\0'
      sprintf(path, "lib/%s", file->d_name);
      p->handle = dlopen(path, RTLD_LAZY);
      free(path);
      if (!p->handle) {
         fprintf(stderr, "%s\n", dlerror());
         exit(EXIT_FAILURE);
      }
      dlerror();

      // load init (which "CREATE TABLE IF NOT EXISTS")
      *(void **) (&p->cmd) = dlsym(p->handle, "init");
      if ((error = dlerror()) != NULL)  {
         fprintf(stderr, "%s\n", error);
         exit(EXIT_FAILURE);
      }
      sql_exec((p->cmd)());

      // load intervall
      *(void **) (&default_interval) = dlsym(p->handle, "default_interval");
      if ((error = dlerror()) != NULL)  {
         fprintf(stderr, "%s\n", error);
         exit(EXIT_FAILURE);
      }
      p->interval = (default_interval)();
      // set minimal interval to 5 sec.
      if ( p->interval < 5 ) {
         p->interval = 5;
      }

      // load run-funtion (save in cmd)
      *(void **) (&p->cmd) = dlsym(p->handle, "run");
      if ((error = dlerror()) != NULL)  {
         fprintf(stderr, "%s\n", error);
         exit(EXIT_FAILURE);
      }
      // init sem for locking
      char str[sizeof(void*)*2+1];
      snprintf(str, (sizeof(void*)*2+10), "/moonit%p", p->cmd);
      // overwrite 0x infront of the pointer
      *(str+7) = 'o';
      *(str+8) = 'r';
      p->lock = sem_open(str,O_CREAT); 
      
      // add plugin "p" to que and plugin-list
      que->plugins[i] = p;
      plugins[i] = p;
      i++;
   }
   // close pointer-array
   que->plugins[i] = NULL;
   plugins[i] = NULL;
}

