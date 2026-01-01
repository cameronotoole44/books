CC=gcc
CFLAGS=-std=c11 -O2 -Wall -Wextra -Wpedantic -Isrc -Ivendor/pdcurses
TARGET=booktracker

SRC=$(wildcard src/*.c)

PDC_SRC=$(wildcard vendor/pdcurses/pdcurses/*.c) \
        $(wildcard vendor/pdcurses/wincon/*.c)

OBJ=$(SRC:.c=.o) $(PDC_SRC:.c=.o)

LDFLAGS=-luser32 -lgdi32

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJ)

.PHONY: all clean

