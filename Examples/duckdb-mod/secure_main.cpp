//
// Created by Sam Ansmink on 29/06/2020.
//

#include <chrono>
#include <cmath>
#include <iostream>
#include <numeric>
#include <vector>

#include "duckdb.h"
#include "stdio.h"

// Used to envoke shim syscalls to do malloc outside SGX
extern "C" {
#include "shim_unistd.h"
}
// Used to run a baseline experiment
//#define unsecure_malloc(size) 0
//#define unsecure_free(size) 0


#define TOTAL_BUFSIZE 1024000000 * 2
#define REPS  5

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
    if (duckdb_query(con, "CREATE TABLE integers(i INTEGER, j INTEGER);", NULL) == DuckDBError) {
    }
    if (duckdb_query(con, "INSERT INTO integers VALUES (3, 4), (5, 6), (7, NULL);", NULL) ==
        DuckDBError) {
    }
    if (duckdb_query(con, "SELECT * FROM integers", &result) == DuckDBError) {
        printf("Error");
    }

    for (size_t row_idx = 0; row_idx < result.row_count; row_idx++) {
        for (size_t col_idx = 0; col_idx < result.column_count; col_idx++) {
            char* val = duckdb_value_varchar(&result, col_idx, row_idx);
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

void benchmark_buffer(unsigned long num_loops, unsigned long buf_size, int* buf) {
    for (unsigned long i = 0; i < num_loops; ++i) {
        for (unsigned long j = 0; j < buf_size; ++j) {
            buf[j] += 1;
        }
    }
}

void run_benchmark(char* buf, unsigned long buf_size) {
    printf("buf_size %ld, ", buf_size);
    std::vector<double> values;

    for (int i = 0; i <= REPS; ++i) {
        int num_loops = (int)(TOTAL_BUFSIZE / buf_size);

        if (num_loops * buf_size != TOTAL_BUFSIZE) {
            printf("Integer division failure");
            return;
        }
        auto start    = std::chrono::steady_clock::now();

        // Benchark runs on ints
        benchmark_buffer(num_loops, (buf_size / 4), (int*)buf);

        auto end = std::chrono::steady_clock::now();

        std::chrono::duration<double> elapsed_seconds = end - start;
        values.push_back(elapsed_seconds.count());
    }
    double sum    = std::accumulate(values.begin(), values.end(), 0.0);
    double mean   = sum / values.size();
    double sq_sum = std::inner_product(values.begin(), values.end(), values.begin(), 0.0);
    double stdev  = std::sqrt(sq_sum / values.size() - mean * mean);

    printf("average %f, ", mean);
    printf("stdev %f", stdev);
}

int main() {
    //  test_duckdb();

    int size = 1*1000*1000;

    while (size < TOTAL_BUFSIZE) {

        auto buf     = (char*)unsecure_malloc(size);
        auto buf_sec = (char*)malloc(size);


        if (buf) {
            printf("unsecure ");
            run_benchmark(buf, size);
            printf("\n");
        }

        if (buf_sec) {
            printf("secure ");
            run_benchmark(buf_sec, size);
            printf("\n");
        }
        unsecure_free(buf);
        unsecure_free(buf_sec);

        size*=2;
    }
    return 0;
}