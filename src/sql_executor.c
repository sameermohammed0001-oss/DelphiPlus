
#include "sql_executor.h"
#include "error.h"

#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static sqlite3 *database = NULL;
static int database_error_reported = 0;

static void close_database(void)
{
    if (database != NULL) {
        sqlite3_close(database);
        database = NULL;
    }
}

static int open_database(void)
{
    if (database != NULL)
        return 1;

    int result = sqlite3_open("delphiplus.db", &database);

    if (result != SQLITE_OK) {
        fprintf(stderr, "SQLite error: %s\n",
                database != NULL
                    ? sqlite3_errmsg(database)
                    : "Could not allocate database connection");

        if (database != NULL) {
            sqlite3_close(database);
            database = NULL;
        }

        error_set("Could not open SQLite database", 0, 0);
        return 0;
    }

    sqlite3_busy_timeout(database, 5000);
    atexit(close_database);

    return 1;
}

typedef struct QueryOutput {
    int headers_printed;
    int rows_printed;
} QueryOutput;

static int print_row(void *data, int column_count,
                     char **values, char **column_names)
{
    QueryOutput *output = (QueryOutput *)data;

    if (!output->headers_printed) {
        for (int i = 0; i < column_count; i++) {
            if (i > 0)
                printf(" | ");

            printf("%s", column_names[i]);
        }

        putchar('\n');

        for (int i = 0; i < column_count; i++) {
            if (i > 0)
                printf("-+-");

            for (size_t j = 0; j < strlen(column_names[i]); j++)
                putchar('-');
        }

        putchar('\n');
        output->headers_printed = 1;
    }

    for (int i = 0; i < column_count; i++) {
        if (i > 0)
            printf(" | ");

        printf("%s", values[i] != NULL ? values[i] : "NULL");
    }

    putchar('\n');
    output->rows_printed++;

    return 0;
}

int sql_execute(const char *sql)
{
    if (sql == NULL)
        return 1;

    while (isspace((unsigned char)*sql))
        sql++;

    if (*sql == '\0')
        return 1;

    if (!open_database())
        return 0;

    QueryOutput output = {0, 0};
    char *error_message = NULL;

    int result = sqlite3_exec(
        database,
        sql,
        print_row,
        &output,
        &error_message
    );

    if (result != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n",
                error_message != NULL
                    ? error_message
                    : sqlite3_errmsg(database));

        sqlite3_free(error_message);

        if (!database_error_reported) {
            error_set("SQL statement execution failed", 0, 0);
            database_error_reported = 1;
        }

        return 0;
    }

    if (output.rows_printed > 0) {
        printf("(%d row%s returned)\n",
               output.rows_printed,
               output.rows_printed == 1 ? "" : "s");
    } else {
        puts("SQL executed successfully.");
    }

    return 1;
}
