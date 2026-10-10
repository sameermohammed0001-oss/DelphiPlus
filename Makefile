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

# OS Detection
ifeq ($(OS),Windows_NT)
	CC = gcc
	TARGET = build/delphiplus.exe
	MKDIR = if not exist build mkdir build
	RM = del /Q /F src\*.o $(subst /,\,$(TARGET)) 2>nul || exit 0
	RUN_CMD = .\build\delphiplus.exe examples\variables.sql
else
	CC = cc
	TARGET = build/delphiplus
	MKDIR = mkdir -p build
	RM = rm -f src/*.o $(TARGET)
	RUN_CMD = ./$(TARGET) examples/variables.sql
endif

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@$(MKDIR)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@$(RM)

run: all
	$(RUN_CMD)