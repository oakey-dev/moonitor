#include <stdio.h>
#include <string.h>

// size for loadavg 46 + 3*(7+3)-1 + 2
#define LOADAVG_BUFFER_SIZE 78

char* init() {
   return "CREATE TABLE IF NOT EXISTS cpu_avg (\
             time  TIMESTAMP PRIMARY KEY DEFAULT CURRENT_TIMESTAMP,\
             avg1  INTEGER,\
             avg5  INTEGER,\
             avg15 INTEGER\
          );";
}

int default_interval() {
   return 60;
}

char* run() {
   int spaces=0, i = 0; // counters
   char* current;
   FILE* pFile;
   char* buffer = malloc(LOADAVG_BUFFER_SIZE*sizeof(char));
   // return if malloc returns NULL
   if ( buffer == NULL ) { 
      fprintf(stderr, "Error in cpu_avg: not enough memory\n");
      return NULL;
   }

   // open /proc/loadavg for reading
   pFile = fopen("/proc/loadavg", "r");
   if ( pFile == NULL ) {
      fprintf(stderr, "Error in cpu_avg: can't open file /proc/loadavg\n");
      return NULL;
   }
   
   //buffer = "INSERT INTO cpu_kram (timestamp,col1,col2,col3) (time(),"
   strcpy(buffer, "INSERT INTO cpu_avg (avg1,avg5,avg15) VALUES (");

   // try to read file
   current = buffer+46; 
   if ( fgets(current, LOADAVG_BUFFER_SIZE, pFile) == NULL ) {
      fprintf(stderr, "Error in cpu_avg: can't open read file /proc/loadavg\n");
      return NULL;
   }

   // close pFile
   fclose(pFile);

   // parse content
   for ( i=0; i<LOADAVG_BUFFER_SIZE; i++ ) {
      if ( *current == ' ' ) {
         spaces++;
         if ( spaces == 3 )
            break;
         *current = ',';
      }
      current++;
   }
   
   // error with content (does not contain 3 values)
   if(spaces != 3) {
      fprintf(stderr, "Error in cpu_avg: reading /proc/loadavg invalid content?\n");
      return NULL;
   }

   // close string
   *current = ')'; current++;
   *current = ';'; current++;
   *current = '\0';

   return buffer;
}
