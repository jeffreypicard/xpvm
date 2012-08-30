/*
 * block_test.c
 *
 * Programs to test the block macros for the XPVM.
 *
 * Author: Jeffrey Picard
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define BLOCK_OWNER( b ) *(uint64_t*)(b - 8)
#define BLOCK_ANNOTS( b ) *(uint64_t*)(b - 16)
#define BLOCK_AUX_LENGTH( b ) *(uint32_t*)(b - 20)
#define BLOCK_OUT_SYM_REFS( b ) *(uint32_t*)(b - 24)
#define BLOCK_EXCEPT_HANDLERS( b ) *(uint32_t*)(b - 28)
#define BLOCK_FRAME_SIZE( b ) *(uint32_t*)(b - 32)
#define BLOCK_LENGTH( b ) *(uint32_t*)(b - 36)

#define BLOCK_HEADER_LENGTH 36

int main( int argc, char **argv )
{
  uint8_t *b = calloc( BLOCK_HEADER_LENGTH, sizeof(uint8_t) );
  b = b + BLOCK_HEADER_LENGTH;

  BLOCK_OWNER( b ) = 0x10;
  BLOCK_LENGTH( b ) = 0x02010;

  fprintf( stderr, "owner: %llx\n", BLOCK_OWNER( b ) );
  fprintf( stderr, "length: %x\n", BLOCK_LENGTH( b ) );

  b = b - BLOCK_HEADER_LENGTH;
  free( b );
  return 0;
}
