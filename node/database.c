#include <stdio.h>
#include <sqlite3.h>

#include "daemon.h"
#include "database.h"

sqlite3* sqlite_db;

int sql_open() {
   if( sqlite3_open(DEFAULT_PATH, &sqlite_db) ){
      fprintf(stderr, "Critical: Can't open database: %s\n", sqlite3_errmsg(sqlite_db));
      sqlite3_close(sqlite_db);
      return 1;
   }
   return 0;
}

void sql_close() {
   sqlite3_close(sqlite_db);
}

int sql_exec(char* sql) {
   char* errMsg = NULL;
   if ( sqlite3_exec(sqlite_db, sql, NULL, 0, &errMsg) != SQLITE_OK ) {
      fprintf(stderr, "SQL error: %s\n", errMsg);
      sqlite3_free(errMsg);
      return 1;
   }
   return 0;
}
