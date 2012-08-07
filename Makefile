#
# Makefile for running xpvm
#

CC = gcc
CFLAGS = -g -Wall -pthread

all: ret_42

ret_42: xpvm.o ret_42.o ret_42.obj
	$(CC) $(CFLAGS) xpvm.o ret_42.o -o ret_42

xpvm.o: xpvm.c xpvm.h
	$(CC) $(CFLAGS) -c xpvm.c

ret_42.o: ret_42.c xpvm.h
	$(CC) $(CFLAGS) -c ret_42.c

inst_disassemble:

clean:
	-rm -f xpvm.o inst_disassemble ret_42 ret_42.o

