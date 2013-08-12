#ifndef _DATABASE_H_
#define _DATABASE_H_

int sql_open();
void sql_close();

// for first version / tests
int sql_exec(char* sql);

// TODO: replace sql_exec with:
int sql_create_table(char** fields);
int sql_insert(char* fields, char* values);

#endif /* _DATABASE_H_ */
