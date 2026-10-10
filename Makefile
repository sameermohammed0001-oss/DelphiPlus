
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

ifeq ($(OS),Windows_NT)

	CC = gcc
	TARGET = build/delphiplus.exe
	MKDIR = if not exist build mkdir build
	RM = del /Q /F src\*.o $(subst /,\,$(TARGET)) 2>nul || exit 0
	RUN_CMD = .\build\delphiplus.exe examples\variables.sql

	SQLITE_CFLAGS ?=
	SQLITE_LIBS ?= -lsqlite3

else

	UNAME_S := $(shell uname -s)
	CC = cc
	TARGET = build/delphiplus
	MKDIR = mkdir -p build
	RM = rm -f src/*.o $(TARGET)
	RUN_CMD = ./$(TARGET) examples/variables.sql

	ifeq ($(UNAME_S),Darwin)
		SQLITE_CFLAGS ?=
		SQLITE_LIBS ?= -lsqlite3
	else
		SQLITE_CFLAGS ?=
		SQLITE_LIBS ?= -lsqlite3
	endif

endif

CFLAGS += $(SQLITE_CFLAGS)
LDLIBS += $(SQLITE_LIBS)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@$(MKDIR)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET) $(LDLIBS)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@$(RM)

run: all
	$(RUN_CMD)
