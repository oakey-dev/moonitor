#include <stdio.h>

#include "daemon.h"
#include "database.h"
#include "collector.h"
#include "listener.h"

int main(int argc, char* argv[]) {
   // TODO: args auswerten

   if ( sql_open() != 0 ) {
      return 1;
   }


   pthread_t last_thread;
   pthread_create(&last_thread,NULL,listener_thread,NULL);
   collector_thread(NULL);
   
   sql_close();
   return 0;
}
