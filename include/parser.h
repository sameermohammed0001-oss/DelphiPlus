
#ifndef DELPHIPLUS_PARSER_H
#define DELPHIPLUS_PARSER_H

#include "lexer.h"
#include "ast.h"

Ast *parser_parse(TokenArray *tokens);

#endif
