#include <stdio.h>

#include "daemon.h"
#include "collector.h"
//#include "listener.h"

int main(int argc, char* argv[]) {
   // TODO: args auswerten

   collector_thread(NULL);
   
   return 0;
}
