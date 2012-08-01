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
 int error_number = 0;

  /* load ret_42.obj, Note: this was written by hand in object code */
  if (!loadObjectFileXPVM("ret_42.obj", &error_number))
  {
    fprintf(stderr, "loadObjectFileXPVM fails with error %d\n", error_number);
    exit(-1);
  }

  return 0;
}
