CFLAGS=-std=c99 -Wall -Wextra -pedantic -ggdb
CFILES=$(shell find src/ -name "*.c")
TARGET=bin/gibson

$(TARGET): $(CFILES)
	mkdir -p $(shell dirname $@)
	$(CC) $^ $(CFLAGS) -o $(TARGET)
