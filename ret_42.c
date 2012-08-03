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
#include "vm520.h"

/* setting for the execute call */
#define TRACE 0

int main( int argc, char **argv )
{
  /* error for functions returning from vm520 */
  int errorNumber = 0;
  int termStatus[1];

  /* load ret_42.obj, Note: this was written by hand in object code */
  if (!loadObjectFileXPVM("ret_42.obj", &errorNumber))
  {
    fprintf(stderr, "loadObjectFileXPVM fails with error %d\n", errorNumber);
    exit(-1);
  }

  executeXPVM( 1, termStatus, 0 );

  return 0;
}
