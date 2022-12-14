CFLAGS=-std=c99 -Wall -Wextra -pedantic -ggdb
SOURCE=gibson.c
TARGET=gibson

.PHONY: all
all:
	$(CC) $(CFLAGS) $(SOURCE) -o $(TARGET)

