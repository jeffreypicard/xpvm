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
uint8_t   *first_block;
uint8_t   *next_block;
uint64_t  xpvm_mem_amt;
uint32_t  num_blocks_allocd;

int malloc_xpvm_init( uint64_t bytes )
{
  allocd_memory = calloc( bytes, sizeof(uint8_t) );
  if( !allocd_memory )
    EXIT_WITH_ERROR("Error: malloc failed in malloc_xpvm_init\n");

  xpvm_mem_amt = bytes;
  next_block = allocd_memory;
  num_blocks_allocd = 0;
  return 1;
}

uint64_t malloc_xpvm( uint32_t bytes )
{
  uint8_t *b = NULL;

  uint8_t *new_next_block = next_block + bytes + BLOCK_HEADER_LENGTH + 
                            (4-(bytes%4)); /* Round to next 4 byte boundary */
  if( xpvm_mem_amt - 4 <= new_next_block - allocd_memory )
    EXIT_WITH_ERROR("Error: malloc_xpvm failed. Out of memory.\n");

  num_blocks_allocd++;

  b = next_block + BLOCK_HEADER_LENGTH;

  next_block = new_next_block;

  add_blk( &blocks, (uint64_t) CAST_INT b );

  return (uint64_t) CAST_INT b;
}

uint64_t malloc_xpvm_native( uint32_t bytes )
{
  pthread_mutex_lock( &malloc_xpvm_mu );
  return malloc_xpvm( bytes );
  pthread_mutex_unlock( &malloc_xpvm_mu );
}
