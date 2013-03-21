#
# Makefile for running xpvm
#

CC := gcc
AS := nasm -f elf64

BUILD := checks
cflags.checks := -g -Wall -pthread -DCHECKS=1
cflags.nchecks := -g -Wall -pthread -DCHECKS=0

CFLAGS := ${cflags.${BUILD}}

all: xpvm

xpvm: xpvm.o obj_file.o opcodes.o allocator.o wrapped_c_lib.so aquire_blk.o
	$(CC) $(CFLAGS) -rdynamic xpvm.o obj_file.o opcodes.o allocator.o aquire_blk.o -o xpvm -ldl

xpvm.o: xpvm.c xpvm.h opcodes.o
	$(CC) $(CFLAGS) -c xpvm.c

opcodes.o: opcodes.c  xpvm.h
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
aquire_blk.o: aquire_blk.asm
	$(AS) $^

.PHONY: clean test

clean:
	-rm -f xpvm xpvm.o obj_file.o opcodes.o allocator.o wrapped_c_lib.o wrapped_c_lib.so aquire_blk.o

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
	./test_files/hex_to_obj ./test_files/opcode_tests.hex ./test_files/opcode_tests.obj
	./xpvm test_files/opcode_tests.obj
	./test_files/hex_to_obj ./test_files/malloc_tests.hex ./test_files/malloc_tests.obj
	./xpvm test_files/malloc_tests.obj
	./test_files/hex_to_obj ./test_files/malloc_test2.hex ./test_files/malloc_test2.obj
	-./xpvm test_files/malloc_test2.obj
	./test_files/hex_to_obj ./test_files/malloc_test3.hex ./test_files/malloc_test3.obj
	./xpvm test_files/malloc_test3.obj
	./test_files/hex_to_obj ./test_files/malloc_test4.hex ./test_files/malloc_test4.obj
	./xpvm test_files/malloc_test4.obj
