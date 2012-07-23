#
# Makefile for running vm520
#

CC = gcc
CFLAGS = -g -Wall -pthread

AS520 = ../as520/as520

all: sumVector pi

sumVector: vm520.o sumVector.o sumVector.obj
	$(CC) $(CFLAGS) vm520.o sumVector.o -o sumVector

pi: vm520.o pi.o pi.obj
	$(CC) $(CFLAGS) vm520.o pi.o -o pi

vm520.o: vm520.c vm520.h
	$(CC) $(CFLAGS) -c vm520.c

sumVector.o: sumVector.c vm520.h
	$(CC) $(CFLAGS) -c sumVector.c

pi.o: pi.c vm520.h
	$(CC) $(CFLAGS) -c pi.c

sumVector.obj: sumVector.asm
	$(AS520) sumVector.asm

pi.obj: pi.asm
	$(AS520) pi.asm

inst_disassemble:

clean:
	-rm -f sumVector.o pi.o vm520.o sumVector pi sumVector.obj pi.obj inst_disassemble

