#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define FLEXQL_OK 0
#define FLEXQL_ERROR 1

// Opaque struct representing the connection
typedef struct FlexQL FlexQL;

// Callback signature (matches SQLite style)
typedef int (*flexql_callback)(void *data, int argc, char **argv, char **azColName);

// Open a connection to the database
int flexql_open(const char *host, int port, FlexQL **db);

// Execute a query
int flexql_exec(FlexQL *db, const char *sql, flexql_callback cb, void *data, char **errMsg);

// Close connection
int flexql_close(FlexQL *db);

// Free memory allocated by flexql (e.g., error messages)
void flexql_free(void *ptr);

#ifdef __cplusplus
}
#endif