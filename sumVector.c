//
// test of vm520 using sumVector.obj
//

#include <stdio.h>
#include <stdlib.h>
#include "vm520.h"

int main(int argc, char *argv[])
{
  // for error returns from vm520 functions
  int errorNumber;

  // does not expect any command-line arguments
  if (argc != 1)
  {
    fprintf(stderr, "command-line arguments ignored!\n");
  }

  // load sumVector.obj
  if (!loadObjectFile("sumVector.obj", &errorNumber))
  {
    fprintf(stderr, "loadObjectFile fails with error %d\n", errorNumber);
    exit(-1);
  }

  // get the addresses of the exported labels
  unsigned int addressSum;
  unsigned int addressTop;
  unsigned int addressDone;
  if (!getAddress("sum", &addressSum))
  {
    fprintf(stderr, "getAddress fails for sum\n");
    exit(-1);
  }
  if (!getAddress("top", &addressTop))
  {
    fprintf(stderr, "getAddress fails for top\n");
    exit(-1);
  }
  if (!getAddress("done", &addressDone))
  {
    fprintf(stderr, "getAddress fails for done\n");
    exit(-1);
  }

  // test the disassemble function (and getWord)
  int wordTop;
  int wordDone;
  char buffer[100];
  if (!getWord(addressTop, &wordTop))
  {
    fprintf(stderr, "getWord fails for top (%u)\n", addressTop);
    exit(-1);
  }
  if (!disassemble(addressTop, buffer, &errorNumber))
  {
    fprintf(stderr, "disassemble fails for top (%d) with error %d\n",
      addressTop, errorNumber);
    exit(-1);
  }
  printf("top:%08x %s\n", addressTop, buffer);
  if (!getWord(addressDone, &wordDone))
  {
    fprintf(stderr, "getWord fails for done (%u)\n", addressDone);
    exit(-1);
  }
  if (!disassemble(addressDone, buffer, &errorNumber))
  {
    fprintf(stderr, "disassemble fails for done (%d) with error %d\n",
      addressDone, errorNumber);
    exit(-1);
  }
  printf("done: %08x %s\n", addressDone, buffer);
 
  // execute the program (with the trace off)
  unsigned int SP[1] = {1000};
  int status[1];
  int wordSum;
  if (!execute(1, SP, status, 0))
  {
    fprintf(stderr, "execute fails\n");
    exit(-1);
  } 
  printf("vm520 halts with status %d\n", status[0]);
  if (!getWord(addressSum, &wordSum))
  {
    fprintf(stderr, "getWord fails for sum (%u)\n", addressSum);
    exit(-1);
  }
  printf("sum is %d\n", wordSum);

  return 0;
}

