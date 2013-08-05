#include <stdio.h>
#include <string.h>
#include "../database.h"

#define LOADAVG_BUFFER_SIZE 128

int init() {
   return eql_exec(
            "CREATE TABLE IF NOT EXISTS cpu_avg (\
               time  TIMESTAMP PRIMARY KEY DEFAULT CURRENT_TIMESTAMP,\
               avg1  INTEGER,\
               avg5  INTEGER,\
               avg15 INTEGER\
            );");
}

int run() {
   int spaces=0, i = 0; // counters
   char* current;
   FILE* pFile;
   char buffer[LOADAVG_BUFFER_SIZE];

   // open /proc/loadavg for reading
   pFile = fopen("/proc/loadavg", "r");
   if ( pFile == NULL ) {
      return 1;
   }
   
   //buffer = "INSERT INTO cpu_kram (timestamp,col1,col2,col3) (time(),"
   strcpy(buffer, "INSERT INTO cpu_kram (avg1,avg5,avg15) VALUES (");

   // try to read file
   current = buffer+48; 
   if ( fgets(current, LOADAVG_BUFFER_SIZE, pFile) == NULL ) {
      return 2;
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
      return 3;
   }

   // close string
   *current = ')'; current++;
   *current = ';'; current++;
   *current = '\0';

   return sql_exec(buffer);
}
