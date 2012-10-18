/*
 * ret_42.c
 *
 * Harness  to test the XPVM with a program that
 * returns 42.
 *
 * Author: Jeffrey Picard
 */
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <pthread.h>
#include "xpvm.h"

/* setting for the execute call */
#define TRACE 0

int main( int argc, char **argv )
{
  /* error for functions returning from vm520 */
  int errorNumber = 0;
  int64_t ptr = 0;
  retStruct *r = NULL;
  uint64_t ret_val = 0;

  /* load ret_42.obj, Note: this was written by hand in object code */
  if (!load_object_file("ret_42_call.obj", &errorNumber))
  {
    fprintf(stderr, "load_object_file fails with error %d\n", errorNumber);
    exit(-1);
  }

  do_init_proc( &ptr, 0, 0, NULL );

  pthread_t *pt = (pthread_t*)(uint32_t)ptr;

  do_proc_join( (uint64_t)(uint32_t)pt, &ret_val );

  r = (retStruct*) (uint32_t) ret_val;

  fprintf( stderr, "r->ret_val: %d\n", (int)r->ret_val );
  fprintf( stderr, "r->status: %d\n", (int)r->status );

  free( r );

  return 0;
}
