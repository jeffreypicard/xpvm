#
# Makefile for running xpvm
#

CC = gcc
CFLAGS = -g -Wall -pthread

all: xpvm

xpvm: xpvm.o obj_file.o opcodes.o allocator.o wrapped_c_lib.so
	$(CC) $(CFLAGS) -rdynamic xpvm.o obj_file.o opcodes.o allocator.o -o xpvm -ldl

xpvm.o: xpvm.c xpvm.h opcodes.o
	$(CC) $(CFLAGS) -c xpvm.c

opcodes.o: opcodes.c xpvm.h
	$(CC) $(CFLAGS) -c opcodes.c

obj_file.o: obj_file.c xpvm.h
	$(CC) $(CFLAGS) -c obj_file.c

allocator.o: allocator.c xpvm.h
	$(CC) $(CFLAGS) -c allocator.c

wrapped_c_lib.so: wrapped_c_lib.o
	$(CC) -fPIC -shared -o wrapped_c_lib.so wrapped_c_lib.o

wrapped_c_lib.o: wrapped_c_lib.c
	$(CC) $(CFLAGS) -fPIC -shared -c wrapped_c_lib.c

# Assembly functions
#print_int.o:	print_int.s
#	$(CC) $(CFLAGS) -c $^

.PHONY: clean test

clean:
	-rm -f xpvm xpvm.o obj_file.o opcodes.o allocator.o wrapped_c_lib.o wrapped_c_lib.so

test:
	#./xpvm test_files/ret_42.obj
	#./xpvm test_files/ret_42_2.obj
	#./xpvm test_files/ret_42_subl_34.obj
	#./xpvm test_files/ret_42_call.obj
	#./xpvm test_files/ret_42_local.obj
	#./xpvm test_files/ret_42_call_local.obj
	#./xpvm test_files/ret_42_malloc.obj
	./xpvm test_files/pi_threaded.obj
	./xpvm test_files/addd_test.obj
