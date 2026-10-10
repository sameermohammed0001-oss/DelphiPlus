
# DelphiPlus

DelphiPlus is a small educational SQL/PLSQL-style interpreter implemented in C.

## Features

- Lexical analysis
- Basic parsing and AST creation
- Variable declarations
- Assignments
- Arithmetic expressions
- DBMS_OUTPUT.PUT_LINE
- Basic semantic checking
- SQL statement collection

## Requirements

- C compiler such as Clang or GCC
- Make

## Build

```bash
make
```

## Run

```bash
./build/delphiplus examples/variables.sql
```

## Clean

```bash
make clean
```

## Limitations

This is a learning project, not a complete Oracle compiler. The SQL executor is a placeholder and does not execute queries against a database. Advanced PL/SQL constructs remain future work.
