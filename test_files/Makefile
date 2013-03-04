#
# Makefile for hex_to_obj
#

CC = gcc
CFLAGS = -g -Wall

all: hex_to_obj

hex_to_obj: hex_to_obj.c
	$(CC) $(CFLAGS) $^ -o hex_to_obj

.PHONY: clean

clean:
	-rm -f hex_to_obj
