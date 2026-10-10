
# DelphiPlus Architecture

DelphiPlus is an educational SQL/PLSQL-style interpreter written in C.

## Processing Pipeline

1. Source file: receives the input program.
2. Lexer: converts source text into tokens.
3. Parser: checks supported syntax.
4. AST: represents the program as nodes.
5. Semantic analyzer: checks variable references.
6. Interpreter: evaluates expressions and executes supported statements.
7. SQL executor: currently prints SQL statements instead of running them on a database.
8. Error handler: displays errors.

## Source Modules

- `src/main.c`: command-line entry point.
- `src/lexer.c`: lexical analysis.
- `src/parser.c`: parsing.
- `src/ast.c`: abstract syntax tree management.
- `src/semantic.c`: semantic checks.
- `src/interpreter.c`: expression evaluation and execution.
- `src/sql_executor.c`: SQL backend placeholder.
- `src/error.c`: diagnostic reporting.
