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

struct _cmdArg
{
  char s[0];
} typedef cmdArg;

struct _retStruct
{
  int status;
  int64_t retVal;
} typedef retStruct;

int main( int argc, char **argv )
{
  /* error for functions returning from vm520 */
  int errorNumber = 0;
  int initStatus = 0;
  int64_t ptr = 0;
  int64_t ptr2 = 0;
  retStruct *r = NULL;
  uint64_t retVal = 0;

  /* load ret_42.obj, Note: this was written by hand in object code */
  if (!loadObjectFileXPVM("ret_42_2.obj", &errorNumber))
  {
    fprintf(stderr, "loadObjectFileXPVM fails with error %d\n", errorNumber);
    exit(-1);
  }

  initStatus = doInitProc( &ptr, 0, 0, NULL );
  //initStatus = doInitProc( &ptr2, 0, 0, NULL );

  pthread_t *pt = (pthread_t*)(uint32_t)ptr;
  //pthread_t *pt2 = (pthread_t*)(uint32_t)ptr2;

  doProcJoin( (uint64_t)(uint32_t)pt, &retVal );

  r = (retStruct*) (uint32_t) retVal;

  fprintf( stderr, "r->retVal: %d\n", (int)r->retVal );
  fprintf( stderr, "r->status: %d\n", (int)r->status );

  free( r );

  //doProcJoin( (uint64_t)(uint32_t)pt2, &retVal );

  //r = (retStruct*) (uint32_t) retVal;

  //fprintf( stderr, "r->retVal: %d\n", (int)r->retVal );
  //fprintf( stderr, "r->status: %d\n", (int)r->status );

  //free( r );

  return 0;
}
