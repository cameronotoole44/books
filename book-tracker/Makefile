CC=gcc
CFLAGS=-std=c11 -O2 -Wall -Wextra -Wpedantic -Isrc
TARGET=booktracker

SRC=$(wildcard src/*.c)
OBJ=$(SRC:.c=.o)

ifeq ($(OS),Windows_NT)
	CFLAGS += -Ivendor/pdcurses
	PDC_SRC=$(wildcard vendor/pdcurses/pdcurses/*.c) $(wildcard vendor/pdcurses/wincon/*.c)
	OBJ += $(PDC_SRC:.c=.o)
	LDFLAGS=-luser32 -lgdi32
else
	LDFLAGS=-lncursesw
endif

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $

clean:
	rm -f $(TARGET) $(OBJ)

.PHONY: all clean
