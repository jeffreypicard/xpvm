#
# Makefile for running xpvm
#

CC = gcc
CFLAGS = -g -Wall -pthread

all: xpvm

xpvm: xpvm.o opcodes.o allocator.o wrapped_c_lib.so print_int.o
	$(CC) $(CFLAGS) -rdynamic xpvm.o opcodes.o allocator.o print_int.o -o xpvm -ldl

xpvm.o: xpvm.c xpvm.h opcodes.o
	$(CC) $(CFLAGS) -c xpvm.c

opcodes.o: opcodes.c xpvm.h
	$(CC) $(CFLAGS) -c opcodes.c

allocator.o: allocator.c xpvm.h
	$(CC) $(CFLAGS) -c allocator.c

wrapped_c_lib.so: wrapped_c_lib.o
	$(CC) -shared -o wrapped_c_lib.so wrapped_c_lib.o

wrapped_c_lib.o: wrapped_c_lib.c
	$(CC) $(CFLAGS) -c wrapped_c_lib.c

# Assembly functions
print_int.o:	print_int.s
	$(CC) $(CFLAGS) -c $^

.PHONY: clean test

clean:
	-rm -f xpvm xpvm.o opcodes.o allocator.o wrapped_c_lib.o wrapped_c_lib.so

test:
	./xpvm ret_42.obj
	./xpvm ret_42_2.obj
	./xpvm ret_42_subl_34.obj
	./xpvm ret_42_call.obj
	./xpvm ret_42_local.obj
	./xpvm ret_42_call_local.obj
	./xpvm ret_42_malloc.obj
