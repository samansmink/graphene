//
// Created by Sam Ansmink on 29/06/2020.
//

#include "duckdb.h"
#include "stdio.h"
#include <iostream>

// Used to envoke shim syscalls to do malloc outside SGX
extern "C" {
    #include "shim_unistd.h"
}

void test_duckdb() {
  printf("Doing duckdb: \n");
  duckdb_database db;
  duckdb_connection con;
  duckdb_result result;

  if (duckdb_open(NULL, &db) == DuckDBError) {
    printf("Error");
  }
  if (duckdb_connect(db, &con) == DuckDBError) {
    printf("Error");
  }
  if (duckdb_query(con, "CREATE TABLE integers(i INTEGER, j INTEGER);", NULL) ==
      DuckDBError) {
  }
  if (duckdb_query(con,
                   "INSERT INTO integers VALUES (3, 4), (5, 6), (7, NULL);",
                   NULL) == DuckDBError) {
  }
  if (duckdb_query(con, "SELECT * FROM integers", &result) == DuckDBError) {
    printf("Error");
  }

  for (size_t row_idx = 0; row_idx < result.row_count; row_idx++) {
    for (size_t col_idx = 0; col_idx < result.column_count; col_idx++) {
      char *val = duckdb_value_varchar(&result, col_idx, row_idx);
      printf("   %s ", val);
      free(val);
    }
    printf("\n");
  }
  duckdb_destroy_result(&result);

  duckdb_disconnect(&con);
  duckdb_close(&db);
  printf("Finished doing duckdb!\n");
}

int main() {

  printf("Hello secure world, lets do some duckdb!\n");

//  test_duckdb();

  auto buf = (char*)unsecure_malloc(100);
  printf("buf[99] (should be X): %c\n", buf[99]);

  auto buf_sec = (char*)malloc(100);
  printf("buf    : %p\n", buf);
  printf("buf_sec: %p\n", buf_sec);

  unsecure_free(buf);
  unsecure_free(buf_sec);
  return 0;
}