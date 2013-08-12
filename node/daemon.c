#include <stdio.h>
#include <string.h>
#include <dlfcn.h>

#include "daemon.h"
#include "database.h"
#include "collector.h"
#include "listener.h"

extern struct plugin** plugins;

int main(int argc, char* argv[]) {
   // TODO: args auswerten

   atexit(stop);

   if ( strcmp(*argv, "./daemon") ) {
      fprintf(stderr, "Error: while development, daemon musst be called with \"./daemon\"\n");
      return 2;
   }

   if ( sql_open() != 0 ) {
      return 1;
   }


   pthread_t last_thread;
   pthread_create(&last_thread,NULL,listener_thread,NULL);
   collector_thread(NULL);
   
   sql_close();
   return 0;
}

void stop(void) {
   int i;
   struct plugin* p;
   // close lib-handles and sem
   for ( i = 0; (p = plugins[i]) != NULL; i++ ) {
      dlclose(p->handle);
      
      char str[sizeof(void*)*2+1];
      snprintf(str, (sizeof(void*)*2+10), "/moonit%p", p->cmd);
      // overwrite 0x infront of the pointer
      *(str+7) = 'o';
      *(str+8) = 'r';
      sem_unlink(str);
   }
}
