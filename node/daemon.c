#include <stdio.h>

#include "daemon.h"
#include "database.h"
#include "collector.h"
//#include "listener.h"

int main(int argc, char* argv[]) {
   // TODO: args auswerten

   if ( sql_open() != 0 ) {
      return 1;
   }


   collector_thread(NULL);
   
   sql_close();
   return 0;
}
