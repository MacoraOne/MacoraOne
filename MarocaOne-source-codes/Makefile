CC ?= cc
CFLAGS ?= -D_POSIX_C_SOURCE=200809L -std=c11 -O2 -Wall -Wextra -Wpedantic
LDFLAGS ?=
LDLIBS ?=

TARGET := one
OBJS := main.o

.PHONY: all clean install uninstall test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

main.o: main.c marocaone.h
	$(CC) $(CFLAGS) -c $< -o $@

install: $(TARGET)
	install -Dm755 $(TARGET) /usr/local/bin/one

uninstall:
	rm -f /usr/local/bin/one

test: $(TARGET)
	./$(TARGET) --version
	./$(TARGET) --help >/dev/null

clean:
	rm -f $(TARGET) $(OBJS)
