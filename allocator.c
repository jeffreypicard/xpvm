/*
 * allocator.c
 *
 * Memory allocator for the XPVM.
 *
 * Author: Jeffrey Picard
 */
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "xpvm.h"

uint8_t   *allocd_memory;
uint8_t   *next_block;
uint64_t  xpvm_mem_amt;

struct _mem_record
{
  uint8_t             *mem;
  block               *id;
  struct _mem_record  *link;
} typedef mem_record;

mem_record *allocd_blocks;

int malloc_xpvm_init( uint64_t bytes )
{
  allocd_memory = calloc( bytes, sizeof(uint8_t) );
  if( !allocd_memory )
    EXIT_WITH_ERROR("Error: malloc failed in malloc_xpvm_init\n");
  allocd_blocks = calloc( 1, sizeof(mem_record) );
  xpvm_mem_amt = bytes;
  next_block = allocd_memory;
  return 1;
}

uint64_t malloc_xpvm( uint32_t bytes )
{
  block *b = calloc( 1, sizeof(block) );
  mem_record *new_rec = calloc( 1, sizeof(mem_record) );
  if( !b || !new_rec )
    EXIT_WITH_ERROR("Error: malloc failed in malloc_xpvm\n");

  uint8_t *new_next_block = next_block + bytes + (4-(bytes%4));
  if( xpvm_mem_amt - 4 <= new_next_block - allocd_memory )
    EXIT_WITH_ERROR("Error: malloc_xpvm failed. Out of memory.\n");

  b->data = next_block;

  allocd_blocks->mem = next_block;
  allocd_blocks->id = b;
 
  new_rec->link = allocd_blocks;
  allocd_blocks = new_rec;

  /* Round up to the next 4-byte boundry */
  next_block = new_next_block;

  return (uint64_t) CAST_INT b;
}
