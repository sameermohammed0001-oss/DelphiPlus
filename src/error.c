
#include "error.h"

#include <stdio.h>

static int errors = 0;

void error_set(const char *message, int line, int column)
{
    fprintf(stderr,
            "DelphiPlus error at %d:%d: %s\n",
            line,
            column,
            message);

    errors++;
}

int error_count(void)
{
    return errors;
}

void error_reset(void)
{
    errors = 0;
}
