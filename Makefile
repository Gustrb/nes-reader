CC=gcc
CFLAGS=-Wall -Wextra -Werror -std=c99 -pedantic
LDFLAGS=-lm
SRC=src/main.c src/file.c src/nes.c
TARGET=main

all: build
	$(CC) $(CFLAGS) $(LDFLAGS) $(SRC) -o build/$(TARGET)

build:
	mkdir build
