
#include "lexer.h"
#include "parser.h"
#include "semantic.h"
#include "interpreter.h"
#include "error.h"

#include <stdio.h>
#include <stdlib.h>

static char *read_file(const char *path)
{
    FILE *file = fopen(path, "rb");

    if (file == NULL) {
        perror(path);
        return NULL;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return NULL;
    }

    long size = ftell(file);

    if (size < 0) {
        fclose(file);
        return NULL;
    }

    rewind(file);

    char *buffer = malloc((size_t)size + 1);

    if (buffer == NULL) {
        fclose(file);
        return NULL;
    }

    size_t bytes_read = fread(buffer, 1, (size_t)size, file);
    buffer[bytes_read] = '\0';

    fclose(file);
    return buffer;
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr,
                "Usage: %s <source.sql>\n",
                argv[0]);

        return 2;
    }

    char *source = read_file(argv[1]);

    if (source == NULL)
        return 2;

    TokenArray tokens;

    if (!lexer_tokenize(source, &tokens)) {
        fprintf(stderr, "Lexer failed to allocate memory.\n");
        free(source);
        return 2;
    }

    Ast *program = parser_parse(&tokens);

    int success = program != NULL && error_count() == 0;

    if (success)
        success = semantic_check(program);

    if (success)
        success = interpreter_run(program);

    ast_free(program);
    lexer_free(&tokens);
    free(source);

    return success ? 0 : 1;
}
