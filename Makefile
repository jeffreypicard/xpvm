#
# Makefile for running xpvm
#

CC = gcc
CFLAGS = -g -Wall -pthread

all: xpvm

ret_42: xpvm.o ret_42.o ret_42.obj
	$(CC) $(CFLAGS) opcodes.o xpvm.o ret_42.o -o ret_42

xpvm: xpvm.o opcodes.o allocator.o
	$(CC) $(CFLAGS) xpvm.o opcodes.o allocator.o -o xpvm

xpvm.o: xpvm.c xpvm.h opcodes.o
	$(CC) $(CFLAGS) -c xpvm.c

opcodes.o: opcodes.c xpvm.h
	$(CC) $(CFLAGS) -c opcodes.c

allocator.o: allocator.c xpvm.h
	$(CC) $(CFLAGS) -c allocator.c

ret_42.o: ret_42.c xpvm.h
	$(CC) $(CFLAGS) -c ret_42.c

inst_disassemble:

clean:
	-rm -f xpvm.o inst_disassemble ret_42 ret_42.o opcodes.o

