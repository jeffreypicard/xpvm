/*
 * wrapped_c_lib.c
 *
 * Code file for the functions wrapping c library
 * functions in order to make them available
 * as native functions to the XPVM.
 *
 * Author: Jeffrey Picard.
 */

#include <stdio.h>
#include <stdint.h>

#include "xpvm.h"

int print_int_a( uint64_t*, int );

/*
 * FIXME:
 * This needs to be converted from an assembly function into
 * one that just uses a little assembly to build the stack
 * then does C function pointer trickery to call printf
 * with no arguments and start off the execution.
 */
int print_int( unsigned int proc_id, uint64_t *reg, stack_frame **stak,
                    uint8_t ri, uint8_t rj, uint8_t c3, uint8_t const8 )
{
  /*if( 0 >= fprintf( stderr, "%d\n", const8 ) )
    return 0;*/
  //char *s = "The answer is %d\n";
  //reg[1] = const8;
  //reg[2] = (uint64_t) CAST_INT s;
  /*fprintf( stderr, "reg: %p\n", reg );*/
  //print_int_a( reg, 2 );
  printf("%d\n", const8 );
  return 1;
}

int xpvm_printf( unsigned int proc_id, uint64_t *reg, stack_frame **stak,
                    uint8_t ri, uint8_t rj, uint8_t c3, uint8_t const8 )
{
  return 1;
}
