#ifndef __DATAQUERY_H__
#define __DATAQUERY_H__
#include <mysql/mysql.h>
typedef struct _QueryRes {
  unsigned int num_fields;
  unsigned long num_rows;
  MYSQL_RES *result;
} QueryRes;

typedef enum Database { Login, Company } Database;

QueryRes *dbquery(const char *query, Database db);
#endif
