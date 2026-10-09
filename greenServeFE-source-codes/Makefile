CC ?= cc
CFLAGS ?= -D_POSIX_C_SOURCE=200809L -std=c11 -O2 -Wall -Wextra -Wpedantic
LDFLAGS ?=
LDLIBS ?= -lm -ldl

TARGET := greenServeFE

OBJS := \
	main.o \
	lexer.o \
	parser.o \
	value.o \
	bytecode.o \
	compiler.o \
	vm.o

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

main.o: main.c lexer.h parser.h compiler.h vm.h
	$(CC) $(CFLAGS) -c $< -o $@

lexer.o: lexer.c lexer.h
	$(CC) $(CFLAGS) -c $< -o $@

parser.o: parser.c parser.h lexer.h
	$(CC) $(CFLAGS) -c $< -o $@

value.o: value.c value.h bytecode.h
	$(CC) $(CFLAGS) -c $< -o $@

bytecode.o: bytecode.c bytecode.h value.h
	$(CC) $(CFLAGS) -c $< -o $@

compiler.o: compiler.c compiler.h parser.h bytecode.h
	$(CC) $(CFLAGS) -c $< -o $@

vm.o: vm.c vm.h bytecode.h value.h lexer.h module.h
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
