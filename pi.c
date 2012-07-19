//
// test of vm520 using pi.obj
//

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "vm520.h"

// setting for the execute call
#define TRACE 0

// number of intervals to process on the x-axis
//   this must be evenly divisible by the number of processors
//   note 10000 is evenly divisible by 16, 8, 4, 2, 1
#define INTERVALS 10000

int main(int argc, char *argv[])
{
  // for error returns from vm520 functions
  int errorNumber;

  // expects the number of processors to be given on the command line
  if ((argc != 2) || (argv[1][0] == 0))
  {
    fprintf(stderr, "usage: pi numberOfprocessors\n");
    exit(-1);
  }

  // get the specified number of processors
  errno = 0;
  char *p;
  long int processors = strtol(argv[1], &p, 10);
  if ((*p != 0) || errno || (processors < 0) || (processors > 16))
  {
    fprintf(stderr, "%s is not a valid number of processors\n", argv[1]);
    exit(-1);
  }

  // load pi.obj
  if (!loadObjectFile("pi.obj", &errorNumber))
  {
    fprintf(stderr, "loadObjectFile fails with error %d\n", errorNumber);
    exit(-1);
  }

  // get the addresses of the exported labels
  unsigned int addressFour;
  unsigned int addressOne;
  unsigned int addressOneHalf;
  unsigned int addressWidth;
  unsigned int addressIntervals;
  unsigned int addressAnswer;
  if (!getAddress("four", &addressFour))
  {
    fprintf(stderr, "getAddress fails for four\n");
    exit(-1);
  }
  if (!getAddress("one", &addressOne))
  {
    fprintf(stderr, "getAddress fails for one\n");
    exit(-1);
  }
  if (!getAddress("oneHalf", &addressOneHalf))
  {
    fprintf(stderr, "getAddress fails for oneHalf\n");
    exit(-1);
  }
  if (!getAddress("intervals", &addressIntervals))
  {
    fprintf(stderr, "getAddress fails for intervals\n");
    exit(-1);
  }
  if (!getAddress("width", &addressWidth))
  {
    fprintf(stderr, "getAddress fails for width\n");
    exit(-1);
  }
  if (!getAddress("answer", &addressAnswer))
  {
    fprintf(stderr, "getAddress fails for answer\n");
    exit(-1);
  }

  // put initial values into the exported symbols
  float x;
  int intervals = INTERVALS;
  x = 1.0;
  if (!putWord(addressOne, * (int *) &x))
  {
    fprintf(stderr, "putWord fails for one (%d)\n", addressOne);
    exit(-1);
  }
  x = 4.0;
  if (!putWord(addressFour, * (int *) &x))
  {
    fprintf(stderr, "putWord fails for four (%d)\n", addressFour);
    exit(-1);
  }
  x = 0.5;
  if (!putWord(addressOneHalf, * (int *) &x))
  {
    fprintf(stderr, "putWord fails for oneHalf (%d)\n", addressOneHalf);
    exit(-1);
  }
  if (!putWord(addressIntervals, intervals))
  {
    fprintf(stderr, "putWord fails for intervals (%d)\n", addressIntervals);
    exit(-1);
  }
  x = 1.0 / intervals;
  if (!putWord(addressWidth, * (int *) &x))
  {
    fprintf(stderr, "putWord fails for width (%d)\n", addressWidth);
    exit(-1);
  }
 
  // execute the program
  unsigned int SP[processors];
  int status[processors];
  int wordAnswer;
  int i;
  int nextSP = 1000;
  for (i = 0; i < processors; i++)  // set initial SP values
  {
    SP[i] = nextSP;
    nextSP += 1000;
  }
  printf("starting vm520 with %ld processors\n", processors);
  if (!execute(processors, SP, status, TRACE))
  {
    fprintf(stderr, "execute fails\n");
    exit(-1);
  } 
  for (i = 0; i < processors; i++)
  {
    printf("processor %d halted with status %d\n", i, status[i]);
  }
  if (!getWord(addressAnswer, &wordAnswer))
  {
    fprintf(stderr, "getWord fails for answer (%u)\n", addressAnswer);
    exit(-1);
  }
  printf("answer is %8.6f\n", * (float *) &wordAnswer);

  return 0;
}

