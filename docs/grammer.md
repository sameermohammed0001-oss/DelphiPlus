
# DelphiPlus Starter Grammar

```ebnf
program       = [ declaration_section ], block ;

declaration_section
              = "DECLARE", { declaration } ;

declaration   = identifier, datatype,
                [ ":=", expression ], ";" ;

datatype      = "NUMBER"
              | "INTEGER"
              | "PLS_INTEGER"
              | "VARCHAR2"
              | "BOOLEAN" ;

block         = "BEGIN", { statement },
                "END", ";" ;

statement     = assignment
              | print_statement
              | sql_statement
              | block ;

assignment    = identifier, ":=", expression, ";" ;

print_statement
              = "DBMS_OUTPUT", ".", "PUT_LINE",
                "(", expression, ")", ";" ;

expression    = primary, { operator, primary } ;

primary       = number
              | string
              | identifier
              | "TRUE"
              | "FALSE"
              | "NULL"
              | "(", expression, ")" ;
```

This grammar describes the intended starter subset, not the complete Oracle PL/SQL grammar.
```
