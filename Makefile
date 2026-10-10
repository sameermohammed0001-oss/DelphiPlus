
CC = cc
CFLAGS = -std=c11 -Wall -Wextra -pedantic -Iinclude -D_POSIX_C_SOURCE=200809L

SOURCES = \
	src/main.c \
	src/lexer.c \
	src/parser.c \
	src/ast.c \
	src/semantic.c \
	src/interpreter.c \
	src/sql_executor.c \
	src/error.c

OBJECTS = $(SOURCES:.c=.o)
TARGET = build/delphiplus

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJECTS)
	mkdir -p build
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(TARGET)

run: all
	./$(TARGET) examples/variables.sql
