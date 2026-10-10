#include "sql_executor.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>


int sql_execute(const char *sql)
{
    if (sql == NULL)
        return 0;

    while (isspace((unsigned char)*sql))
        sql++;

    if (*sql == '\0')
        return 1;

    if (strncasecmp(sql, "COMMIT", 6) == 0) {
        puts("Transaction committed (simulated).");
        return 1;
    }

    if (strncasecmp(sql, "ROLLBACK", 8) == 0) {
        puts("Transaction rolled back (simulated).");
        return 1;
    }

    printf("[SQL backend not connected] %s\n", sql);
    return 1;
}
