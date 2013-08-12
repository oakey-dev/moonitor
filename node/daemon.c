#include <stdio.h>
#include <string.h>
#include <dlfcn.h>

#include "daemon.h"
#include "database.h"
#include "collector.h"
#include "listener.h"

int main(int argc, char* argv[]) {
   // TODO: args auswerten


   if ( strcmp(*argv, "./daemon") ) {
      fprintf(stderr, "Error: while development, daemon musst be called with \"./daemon\"\n");
      return 2;
   }

   if ( sql_init() != 0 ) {
      return 1;
   }


   pthread_t last_thread;
   pthread_create(&last_thread,NULL,listener_thread,NULL);
   collector_thread(NULL);
   
   sql_close();
   return 0;
}
