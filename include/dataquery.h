#ifndef __DATAQUERY_H__
#define __DATAQUERY_H__
#include <mysql/mysql.h>
typedef struct _QueryRes QueryRes;

typedef enum Database { Login, Company } Database;

QueryRes *dbquery(const char *query, Database db);
#endif
