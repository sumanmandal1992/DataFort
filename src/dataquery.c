#include "dataquery.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct _QueryRes {
  unsigned int num_fields;
  unsigned long num_rows;
  MYSQL_RES *result;
} QueryRes;

typedef struct _Pool {
  const char *host;
  const char *usr;
  const char *passwd;
  const char *db;
} Pool;

static Pool *create_pool(const char *file);
static void *connect_db(const char *file);
static void finish_with_error(MYSQL *con);

static Pool *create_pool(const char *file) {
  Pool *pool = (Pool *)malloc(sizeof(Pool));

  FILE *fp = fopen(file, "r");
  if (fp == NULL) {
    char errmsg[50];
    sprintf(errmsg, "Unable to open file");
    perror(errmsg);
    return NULL;
  }

  bool key = true, val = false;
  int i = 0, j = 0, ch;
  char k[20], v[1000];
  while ((ch = fgetc(fp)) != EOF) {

    if (ch == '\n') {
      k[i] = '\0';
      v[j] = '\0';
      char *tmp = (char *)malloc(sizeof(char) * strlen(v) + 1);
      int t;
      for (t = 0; t < (int)strlen(v); t++)
        tmp[t] = v[t];
      tmp[t] = '\0';

      if (strcmp(k, "usr") == 0) {
        if (strcmp(tmp, "") == 0)
          pool->usr = NULL;
        else
          pool->usr = (const char *)tmp;
      } else if (strcmp(k, "passwd") == 0) {
        if (strcmp(tmp, "") == 0)
          pool->passwd = NULL;
        else
          pool->passwd = (const char *)tmp;
      } else if (strcmp(k, "host") == 0) {
        if (strcmp(tmp, "") == 0)
          pool->host = NULL;
        else
          pool->host = (const char *)tmp;
      } else if (strcmp(k, "db") == 0) {
        if (strcmp(tmp, "") == 0)
          pool->db = NULL;
        else
          pool->db = (const char *)tmp;
      }

      i = j = 0;
      key = true;
      val = false;
    } else {
      if (!key)
        val = true;
      if (ch == '=')
        key = false;
      if (key)
        k[i++] = ch;
      if (val)
        v[j++] = ch;
    }
  }
  fclose(fp);

  return pool;
}

static void finish_with_error(MYSQL *con) {
  fprintf(stderr, "%s\n", mysql_error(con));
  mysql_close(con);
}

static void *connect_db(const char *file) {
  MYSQL *con = mysql_init(NULL);
  if (con == NULL) {
    fprintf(stderr, "%s\n", mysql_error(con));
    return NULL;
  }

  Pool *pool = create_pool(file);
  if (mysql_real_connect(con, pool->host, pool->usr, pool->passwd, pool->db, 0,
                         NULL, 0) == NULL) {
    finish_with_error(con);
    return NULL;
  }

  free((char *)pool->db);
  free((char *)pool->usr);
  free((char *)pool->passwd);
  free((char *)pool->host);
  free(pool);

  return con;
}

/*
 * Query database
 */
QueryRes *dbquery(const char *query, Database db) {

  const char *file = NULL;
  if (db == Login) {
    file = ".logenv";
  } else if (db == Company) {
    file = ".compenv";
  }

  MYSQL *con = connect_db(file);
  QueryRes *res = (QueryRes *)malloc(sizeof(QueryRes));

  if (con == NULL) {
    printf("Failed to connect...\n");
    return NULL;
  }

  if (res == NULL) {
    printf("Failed for create result set...\n");
    return NULL;
  }

  // For db query
  if (mysql_query(con, query)) {
    finish_with_error(con);
    return NULL;
  }

  MYSQL_RES *result = mysql_store_result(con);

  if (result == NULL) {
    finish_with_error(con);
    return NULL;
  }

  int num_fields = mysql_num_fields(result);
  int num_rows = mysql_num_rows(result);

  // MYSQL_ROW row = NULL;
  // while ((row = mysql_fetch_row(result))) {
  //   for (int i = 0; i < num_fields; i++) {
  //     printf("%s ", row[i] ? row[i] : "NULL");
  //   }

  //   printf("\n");
  // }

  // mysql_free_result(result);
  res->num_fields = num_fields;
  res->num_rows = num_rows;
  res->result = result;

  mysql_close(con);
  return res;
}
