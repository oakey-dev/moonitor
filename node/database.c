#include <stdio.h>
#include <sqlite3.h>

#include "daemon.h"
#include "database.h"

sqlite3* sqlite_db;

int sql_init() {
   if ( sql_open() != 0 ) {
      return 1;
   }
   if ( sql_exec("CREATE TABLE IF NOT EXISTS plugin (\
                     id VARCHAR(16),\
                     category VARCHAR(32),\
                     title VARCHAR(64),\
                     description VARCHAR(256),\
                     desc_x VARCHAR(64),\
                     unit_x VARCHAR(8),\
                     desc_y VARCHAR(64),\
                     unit_y VARCHAR(8),\
                     base_y INT DEFAULT 1024,\
                     min_y INT DEFAULT 0,\
                     max_y INT DEFAULT 0,\
                     PRIMARY KEY (id)\
                 )") != 0 ) {
      return 2;
   }
   if ( sql_exec("CREATE TABLE IF NOT EXISTS option (\
                     id VARCHAR(16),\
                     field VARCHAR(16),\
                     draw TINYINT DEFAULT 1,\
                     type TINYINT DEFAULT 1,\
                     warn INT DEFAULT 0,\
                     crit INT DEFAULT 0,\
                     PRIMARY KEY (id, field)\
                 )") != 0 ) {
      return 2;
   }
   
   return 0;
}


int sql_open() {
   if ( sqlite3_open(DEFAULT_PATH, &sqlite_db) ) {
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

