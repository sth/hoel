#include <stdio.h>
#include <stdlib.h>
#include <jansson.h>
#define _HOEL_SQLITE
#include "../src/hoel.h"

/**
 * Implementation of sprintf that return a malloc'd char *  with the string construction
 * because life is too short to use 3 lines instead of 1
 * but don't forget to free the returned value after use!
 */
char * msprintf(const char * message, ...) {
  va_list argp, argp_cpy;
  size_t out_len = 0;
  char * out = NULL;
  va_start(argp, message);
  va_copy(argp_cpy, argp);
  out_len = vsnprintf(NULL, 0, message, argp);
  out = malloc(out_len+sizeof(char));
  if (out == NULL) {
    return NULL;
  }
  vsnprintf(out, (out_len+sizeof(char)), message, argp_cpy);
  va_end(argp);
  va_end(argp_cpy);
  return out;
}

void print_result(struct _h_result result) {
  int col, row, i;
  printf("rows: %d, col: %d\n", result.nb_rows, result.nb_columns);
  for (row = 0; row<result.nb_rows; row++) {
    for (col=0; col<result.nb_columns; col++) {
      switch(result.data[row][col].type) {
        case HOEL_COL_TYPE_INT:
          printf("| %d ", ((struct _h_type_int *)result.data[row][col].t_data)->value);
          break;
        case HOEL_COL_TYPE_DOUBLE:
          printf("| %f ", ((struct _h_type_double *)result.data[row][col].t_data)->value);
          break;
        case HOEL_COL_TYPE_TEXT:
          printf("| %s ", ((struct _h_type_text *)result.data[row][col].t_data)->value);
          break;
        case HOEL_COL_TYPE_BLOB:
          for (i=0; i<((struct _h_type_blob *)result.data[row][col].t_data)->length; i++) {
            printf("%c", *((char*)(((struct _h_type_blob *)result.data[row][col].t_data)->value+i)));
            if (i%80 == 0 && i>0) {
              printf("\n");
            }
          }
          break;
        case HOEL_COL_TYPE_NULL:
          printf("| null ");
          break;
      }
    }
    printf("|\n");
  }
}

void unit_tests(struct _h_connection * conn) {
  json_t * j_result;
  struct _h_result result;
  struct _h_data * data;
  char * query = NULL, * sanitized = NULL, * dump, * table = "test";
  int last_id = -1;
  
  query = msprintf("select * from %s", table);
  if (h_query_select_json(conn, query, &j_result) == H_OK) {
    dump = json_dumps(j_result, JSON_INDENT(2));
    printf("json result is\n%s\n", dump);
    json_decref(j_result);
    free(dump);
  } else {
    printf("Error executing query\n");
  }
  free(query);
  
  sanitized = h_escape_string(conn, "Hodor son of H'rtp'ss");
  query = msprintf("insert into %s (name, age, temperature) values ('%s', %d, %f)", table, sanitized, 33, 37.2);
  printf("insert result: %d\n", h_query_insert(conn, query));
  free(sanitized);
  free(query);
  
  query = msprintf("select * from %s", table);
  if (h_query_select(conn, query, &result) == H_OK) {
    print_result(result);
    h_clean_result(&result);
  } else {
    printf("Error executing query\n");
  }
  free(query);
  
  sanitized = h_escape_string(conn, "Ygritte you know nothing");
  query = msprintf(NULL, 0, "insert into %s (name, age, temperature) values ('%s', %d, %f)", table, sanitized, 25, 30.1);
  printf("insert result: %d\n", h_query_insert(conn, query));
  free(sanitized);
  free(query);
  
  query = msprintf("select * from %s", table);
  if (h_query_select(conn, query, &result) == H_OK) {
    print_result(result);
    h_clean_result(&result);
  } else {
    printf("Error executing query\n");
  }
  free(query);
  
  sanitized = h_escape_string(conn, "Littlefinger I will betray you");
  query = msprintf("insert into %s (name, age, temperature) values ('%s', %d, %f)", table, sanitized, 44, 40.5);
  printf("insert result: %d\n", h_query_insert(conn, query));
  free(sanitized);
  free(query);
  data = h_last_insert_id(conn);
  if (data->type == HOEL_COL_TYPE_INT) {
    last_id = ((struct _h_type_int *)data->t_data)->value;
  }
  h_clean_data_full(data);
  printf("last id is %d\n", last_id);
  
  query = msprintf("select * from %s", table);
  if (h_query_select(conn, query, &result) == H_OK) {
    print_result(result);
    h_clean_result(&result);
  } else {
    printf("Error executing query\n");
  }
  free(query);
  
  sanitized = h_escape_string(conn, "Littlefinger I am nothing");
  query = msprintf("update %s set name='%s' where id=%d", table, sanitized, last_id);
  printf("update result: %d\n", h_query_update(conn, query));
  free(sanitized);
  free(query);

  query = msprintf("select * from %s", table);
  if (h_query_select(conn, query, &result) == H_OK) {
    print_result(result);
    h_clean_result(&result);
  } else {
    printf("Error executing query\n");
  }
  free(query);
  
  query = msprintf("delete from %s where id=%d", table, last_id);
  printf("delete result: %d\n", h_query_delete(conn, query));
  free(query);
  
  query = msprintf("select * from %s", table);
  if (h_query_select_json(conn, query, &j_result) == H_OK) {
    dump = json_dumps(j_result, JSON_INDENT(2));
    printf("json result is\n%s\n", dump);
    json_decref(j_result);
    free(dump);
  } else {
    printf("Error executing query\n");
  }
  free(query);
}

int main(int argc, char ** argv) {
  struct _h_connection * conn;
  char * db_file = "/tmp/test.db";
  
  conn = h_connect_sqlite(db_file);
  
  if (conn != NULL) {
    unit_tests(conn);
  }
  h_close_db(conn);
  return h_clean_connection(conn);
}
