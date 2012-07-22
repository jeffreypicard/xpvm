//
// vm520.c
//
// the vm520 implementation
//
// this is Phil Hatcher's implementation
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

#include "vm520.h"

#define MEMORY_SIZE 1048576
#define MAX_ADDRESS (MEMORY_SIZE-1)

// forward references
static int readWord(FILE *fp, int *outWord);
static int readSymbol(FILE *fp, char **outSymbol, unsigned int *outAddr);
static int readObjectCode(FILE *fp, int *memory, int length);
static void cleanupInsymbols(void);
static void *fetchExecute(void *);
static int format1(unsigned int, int, int[]);
static int format2(unsigned int, int, int[]);
static int format3(unsigned int, int, int[]);
static int format4(unsigned int, int, int[]);
static int format5(unsigned int, int, int[]);
static int format6(unsigned int, int, int[]);
static int format7(unsigned int, int, int[]);
static int format8(unsigned int, int, int[]);

// linked list to keep track of (symbol,address) pairs for insymbols
static struct insymbol {
  char *symbol;
  unsigned int addr;
  struct insymbol *next;
} *insymbols = NULL;

// the VM memory
static int memory[MEMORY_SIZE];

// mutex to control access to output log to stderr
static pthread_mutex_t muTrace;

// mutex to control access to the memory bus
static pthread_mutex_t muBus;

// initial SP values
static int initialSP[16];

// is tracing on?
static int doTrace = 0;

// record number of processors in use
static unsigned int numberOfProcessors = 0;

// array used to decode opcodes
//   contains:
//     opcode string
//     format number
//     function to decode and execute the instruction
#define MAX_OPCODE 25
static struct opcodeInfo
{
   char* opcode;
   int format;
   int (*formatFunc)(unsigned int procID, int word, int reg[]);
}
opcodes[] =
{
{"halt",    1, format1}, // 0
{"load",    5, format5}, // 1
{"store",   5, format5}, // 2
{"ldimm",   4, format4}, // 3
{"ldaddr",  5, format5}, // 4
{"ldind",   7, format7}, // 5
{"stind",   7, format7}, // 6
{"addf",    6, format6}, // 7
{"subf",    6, format6}, // 8
{"divf",    6, format6}, // 9
{"mulf",    6, format6}, // 10
{"addi",    6, format6}, // 11
{"subi",    6, format6}, // 12
{"divi",    6, format6}, // 13
{"muli",    6, format6}, // 14
{"call",    2, format2}, // 15
{"ret",     1, format1}, // 16
{"blt",     8, format8}, // 17
{"bgt",     8, format8}, // 18
{"beq",     8, format8}, // 19
{"jmp",     2, format2}, // 20
{"cmpxchg", 8, format8}, // 21
{"getpid",  3, format3}, // 22
{"getpn",   3, format3}, // 23
{"push",    3, format3}, // 24
{"pop",     3, format3}, // 25
};

//////////////////////////////////////////////////////////////////////
// implementation of the public interface to the VM

/*
 * loadObjectFileXPVM
 *
 * Loads an object file for the XPVM
 *
 *  only one object file may be loaded at a time
 *  the function returns 1 if successful and 0 otherwise
 *  if 0 is returned then an error number is returned through the second
 *    parameter if it is not NULL
 *  the following error numbers are supported:
 *    -1: file not found
 *    -2: file contains outsymbols
 *    -3: file is not a valid object file
 */
int loadObjectFileXPVM( char *filename, int *errorNumber )
{
  FILE *fp;
  unsigned int blockCount = 0;
  const unsigned int MAGIC = 0x31303636;
  unsigned int magic = 0;
  int i = 0;

  /* Open the file */
  fp = fopen( filename, "r" );
  if( NULL == fp )
  {
    *errorNumber = -1;
    return 0;
  }

  /* read headers */
  if( !readWord( fp, (int*) &magic ) )
  {
    *errorNumber = -3;
    return 0;
  }
  if( MAGIC != magic )
  {
    *errorNumber = -3;
    return 0;
  }
  if( !readWord( fp, (int *) &blockCount ) )
  {
    *errorNumber = -3;
    return 0;
  }

#if DEBUG_XPVM
  fprintf("blockCount: %d\n", blockCount );
#endif

  return 0;
}

// load an object file
//   only one object file may be loaded at a time
//   the function returns 1 if successful and 0 otherwise
//   if 0 is returned then an error number is returned through the second
//     parameter if it is not NULL
//   the following error numbers are supported:
//     -1: file not found
//     -2: file contains outsymbols
//     -3: file is not a valid object file
int loadObjectFile(char *filename, int *errorNumber)
{
  FILE *fp;
  unsigned int inLength;
  unsigned int outLength;
  unsigned int objLength;
  int i;
  int cnt;

  // first, open the file
  fp = fopen(filename, "r");
  if (fp == NULL)
  {
    *errorNumber = -1;
    return 0;
  }

  // read the headers
  if (!readWord(fp, (int *) &inLength))
  {
    *errorNumber = -3;
    return 0;
  }
  if (!readWord(fp, (int *) &outLength))
  {
    *errorNumber = -3;
    return 0;
  }
  if (!readWord(fp, (int *) &objLength))
  {
    *errorNumber = -3;
    return 0;
  }

  // should not be any outsymbols
  if (outLength != 0)
  {
    *errorNumber = -2;
    return 0;
  }

  // sanity check: insymbol length should be evenly divisible by 5
  if (inLength % 5 != 0)
  {
    *errorNumber = -3;
    return 0;
  }

  // cleanup the old insymbols list
  cleanupInsymbols();

  // read the insymbols and save them
  cnt = inLength / 5;
  for (i = 0; i < cnt; i += 1)
  {
    char *sym;
    unsigned int addr;
    if (!readSymbol(fp, &sym, &addr))
    {
      *errorNumber = -3;
      return 0;
    }
    struct insymbol *p = malloc(sizeof(struct insymbol));
    if (p == NULL)
    {
      fprintf(stderr, "out of memory in loadObjectFile\n");
    }
    p->symbol = sym;
    p->addr = addr;
    p->next = insymbols;
    insymbols = p;
  }

  // read the object code into memory
  if (!readObjectCode(fp, memory, objLength))
  {
    *errorNumber = -3;
    return 0;
  }

  return 1;
}

// get the address of a symbol in the current object file
//   the label must be a symbol in the insymbol section of the object file
//   the address is returned through the second parameter
//   the function returns one if successful and 0 otherwise
int getAddress(char *label, unsigned int *outAddr)
{
  // look up symbol in the insymbol list
  struct insymbol *p = insymbols;
  while (p != NULL)
  {
    // if found return its address
    if (!strcmp(label, p->symbol))
    {
      *outAddr = p->addr;
      return 1;
    }
    p = p->next;
  }
  // if not found then fail
  return 0;
}

// get a word from the memory of the current object file
//   the word is returned through the second parameter
//   the function returns one if successful and 0 otherwise
int getWord(unsigned int addr, int *outWord)
{
  // be sure that the address is in range
  if (addr > MAX_ADDRESS)
  {
    return 0;
  }
  else
  {
    *outWord = memory[addr];
    return 1;
  }
}

// put a word to the memory of the current object file
//   the function returns one if successful and 0 otherwise
int putWord(unsigned int addr, int word)
{
  // be sure that the address is in range
  if (addr > MAX_ADDRESS)
  {
    return 0;
  }
  else
  {
    memory[addr] = word;
    return 1;
  }
}

//   the function returns 1 if all processors are successfully started and
//     0 otherwise
//   the first parameter specifies the number of processors to use
//   the second parameter provides an initial SP value for each processor
//   the third parameter is used to return the termination status for
//     each processor
//   the following termination statuses are supported:
//     0: normal termination
//    -1: divide by zero
//    -2: address out of range
//    -3: illegal instruction
//   the fourth parameter is a Boolean indicating whether an instruction
//     trace should be be printed to stderr
//   Note: that all other registers will be initialized to 0, including
//     the PC and the FP.
int execute(unsigned int numProcessors, unsigned int initSP[],
      int terminationStatus[], int trace)
{
  int i;

  // validate numProcessors
  if ((numProcessors == 0) || (numProcessors > VM520_MAX_PROCESSORS))
  {
    return 0;
  }

  // record number of processors in a global for use by getpn inistruction
  numberOfProcessors = numProcessors;

  // record in a global whether tracing is on or not
  doTrace = trace;

  // allocate storage for thread IDs
  pthread_t pt[numProcessors];

  // copy initial SP values into global array
  for (i = 0; i < numProcessors; i++)
  {
    initialSP[i] = initSP[i];
  }

  // initialize the mutex for controlling access to the trace file
  if (pthread_mutex_init(&muTrace, NULL) != 0)
  {
    perror("can't init mutex");
    return 0;
  }

  // initialize the mutex for controlling access to the memory bus
  if (pthread_mutex_init(&muBus, NULL) != 0)
  {
    perror("can't init mutex");
    return 0;
  }

  // start a thread for each processor
  for (i = 0; i < numProcessors; i++)
  {
    if (pthread_create(&pt[i], NULL, fetchExecute, (void *) i) != 0)
    {
      perror("error in thread create");
      return 0;
    }
  }

  // now use pthread_join to wait for each thread to finish.
  // the return value of the thread indicates whether they halted on an
  // error or not.
  for (i = 0; i < numProcessors; i++)
  {
    void *ret;
    if (pthread_join(pt[i], &ret) != 0)
    {
      perror("error in thread create");
      exit(-1);
    }
    terminationStatus[i] = (int) ret;
  }

  return 1;
}

// disassemble the word at the given address
//   return 1 if the word contains a valid opcode and 0 otherwise
//   the first parameter contains the address of the word to disassemble
//   the second parameter is a pointer to a buffer where the output should be
//     placed
//   the output will be the opcode followed by a space, followed by the
//     comma separated operands (if any). each comma will be followed by
//     a space. PC-relative addresses are converted to absolute addresses
//     and displayed in decimal. offsets and immediate constants are displayed
//     in decimal.
//   the caller can rely that the output will certainly not consume more than
//     100 characters
int disassemble(unsigned int address, char *buffer, int *errorNumber)
{
  // be sure address is in range
  if (address > MAX_ADDRESS)
  {
    *errorNumber = VM520_ADDRESS_OUT_OF_RANGE;
    return 0;
  }

  // get word from memory and get the opcode piece
  int word = memory[address];
  unsigned int opcode = word & 0xFF;

  // is it a valid opcode?
  if (opcode > MAX_OPCODE)
  {
    *errorNumber = VM520_ILLEGAL_INSTRUCTION;
    return 0;
  }
  char *opcodeStr = opcodes[opcode].opcode;
  int format = opcodes[opcode].format;

  // handle each format
  switch (format)
  {
    case 1:
    {
      sprintf(buffer, "%s", opcodeStr);
      break;
    }

    case 2:
    {
      int absAddr = (address + 1) + (word >> 12);
      sprintf(buffer, "%s %d", opcodeStr, absAddr);
      break;
    }

    case 3:
    {
      int reg = (word >> 8) & 0x0F;
      sprintf(buffer, "%s r%d", opcodeStr, reg);
      break;
    }

    case 4:
    {
      int reg = (word >> 8) & 0x0F;
      int constant = word >> 12;
      sprintf(buffer, "%s r%d, %d", opcodeStr, reg, constant);
      break;
    }

    case 5:
    {
      int reg = (word >> 8) & 0x0F;
      int absAddr = (address + 1) + (word >> 12);
      sprintf(buffer, "%s r%d, %d", opcodeStr, reg, absAddr);
      break;
    }

    case 6:
    {
      int reg1 = (word >> 8) & 0x0F;
      int reg2 = (word >> 12) & 0x0F;
      sprintf(buffer, "%s r%d, r%d", opcodeStr, reg1, reg2);
      break;
    }

    case 7:
    {
      int reg1 = (word >> 8) & 0x0F;
      int reg2 = (word >> 12) & 0x0F;
      int offset = word >> 16;
      sprintf(buffer, "%s r%d, %d(r%d)", opcodeStr, reg1, offset, reg2);
      break;
    }

    case 8:
    {
      int reg1 = (word >> 8) & 0x0F;
      int reg2 = (word >> 12) & 0x0F;
      int absAddr = (address + 1) + (word >> 16);
      sprintf(buffer, "%s r%d, r%d, %d", opcodeStr, reg1, reg2, absAddr);
      break;
    }
  }

  return 1;
}

//////////////////////////////////////////////////////////////////////
// functions to read an object file

// readWord
//   read a word from the object file
//   returns word through the second parameter
//   returns 0 if EOF detected
//
//   Reads in 32 bits from a file and stores them into an integer,
//   reversing Endianess in the process. {JP}
static int readWord(FILE *fp, int *outWord)
{
  int c1 = getc(fp);
  int c2 = getc(fp);
  int c3 = getc(fp);
  int c4 = getc(fp);

  if (c4 == EOF)
  {
    return 0;
  }
  *outWord = ((c4 << 24) | (c3 << 16) | (c2 << 8) | c1);
  return 1;
}

// readSymbol
//   read a record from either insymbol or outsymbol table
//   space is malloc-ed for the symbol name
//   (symbol,label) pair is saved for use to implement getAddress
//   symbol name and its address returned via second and third parameters
//   returns 0 if EOF detected
static int readSymbol(FILE *fp, char **outSymbol, unsigned int *outAddr)
{
  char buf[17];
  int i;
  char *retSym;
  unsigned int retAddr;

  // always 16 bytes for symbol name, read them
  for (i = 0; i < 16; i += 1)
  {
    int c = getc(fp);
    if (c == EOF)
    {
      return 0;
    }
    buf[i] = c;
  }

  // slap a null on the end, in case name consumes all 16 bytes
  buf[16] = 0;
 
  // now malloc space and copy the name into it
  retSym = malloc(strlen(buf)+1);
  if (retSym == NULL)
  {
    fprintf(stderr, "out of memory in readSymbol\n");
  }
  strcpy(retSym, buf);

  // now read the address word
  if (!readWord(fp, (int *) &retAddr))
  {
    return 0;
  }

  // now return the symbol name and its address
  *outSymbol = retSym;
  *outAddr = retAddr;

  return 1;
}

// readObjectCode
//   read all object code into memory starting at address 0
//   the second parameter is a pointer to memory word 0
//   the third parameter is the expected length of the object code section
//   returns 1 if successful and 0 otherwise
static int readObjectCode(FILE *fp, int *memory, int length)
{
  int i;

  for (i = 0; i  < length; i += 1)
  {
    int w;
    if (!readWord(fp, &w))
    {
      return 0;
    }
    memory[i] = w;
  }
  return 1;
}

// cleanupInsymbols
//   free up all old insymbol records
static void cleanupInsymbols(void)
{
  struct insymbol *p = insymbols;
  struct insymbol *q;

  while (p)
  {
    q = p->next;
    free(p);
    p = q;
  }

  insymbols = NULL;
}

#define PC reg[15]
#define SP reg[14]
#define FP reg[13]
//////////////////////////////////////////////////////////////////////
// the fetch-execute cycle itself
// this is used as the "work function" for threads
static void *fetchExecute(void *v)
{
  int i;

  // the processor ID is passed in
  int processorID = (int) v;

  // the registers
  int reg[16];
  for (i = 0; i < 16; i += 1)
  {
    reg[i] = 0;
  }
  SP = initialSP[processorID];

  // fetch/execute cycle
  while (1)
  {
    // check to see if the PC is in range
    if ((PC < 0) || (PC >= MAX_ADDRESS))
    {
      return (void *) VM520_ADDRESS_OUT_OF_RANGE;
    }

    // print to the trace if tracing turned on
    if (doTrace)
    {
      // lock the trace mutex
      if (pthread_mutex_lock(&muTrace) != 0)
      {
        fprintf(stderr, "procID %d: pthread_mutex_lock failed for trace!\n",
          processorID);
        exit(-1);
      }

      // display the registers
      fprintf(stderr, "<%d>: %08x %08x %08x %08x %08x %08x %08x %08x\n",
        processorID,
        reg[0], reg[1], reg[2], reg[3], reg[4], reg[5], reg[6], reg[7]);
      fprintf(stderr, "<%d>: %08x %08x %08x %08x %08x %08x %08x %08x\n",
        processorID,
        reg[8], reg[9], reg[10], reg[11], reg[12], reg[13], reg[14], reg[15]);

      // disassemble the next instruction to be executed
      char buffer[100];
      int errNum;
      int ret = disassemble(reg[15], buffer, &errNum);
      if (ret == 0)
      {
        fprintf(stderr,
          "procID %d: disassemble failed (%d) for fetchExecute!\n",
          processorID, errNum);
        exit(-1);
      }
      fprintf(stderr, "<%d>: %s\n", processorID, buffer); 

      // unlock the trace mutex
      if (pthread_mutex_unlock(&muTrace) != 0)
      {
        fprintf(stderr, "procID %d: pthread_mutex_unlock failed for trace!\n",
          processorID);
        exit(-1);
      }
    }

    // fetch
    int word = memory[PC];

    // update PC
    PC += 1;

    // execute
    unsigned char opcode = word & 0xFF;
    if (opcode > MAX_OPCODE) // illegal instruction
    {
      return  (void *) VM520_ILLEGAL_INSTRUCTION;
    }
    int ret = opcodes[opcode].formatFunc(processorID, word, reg);
    if (ret <= 0)
    {
      return (void *) ret;
    }
    else if (ret != 1)
    {
      fprintf(stderr,
        "procID %d: unexpected return value from formatFunc!\n",
        processorID);
      exit(-1);
    }
  }
  
  // won't reach here
  return 0;
}

// decode and execute a format 1 instruction: halt and ret
//   return 1 to indicate continue execution
//   return VM520_NORMAL_TERMINATION to indicate halt instruction was executed
//   return VM520_ADDRESS_OUT_OF_RANGE for bad address in ret instruction
//   
static int format1(unsigned int processorID, int word, int reg[])
{
  unsigned char opcode = word & 0xFF;

  if (opcode == 0) // halt
  {
    return VM520_NORMAL_TERMINATION;
  }
  else if (opcode == 16) // ret
  {
    if (SP < 0 || SP >= MAX_ADDRESS)
    {
      return VM520_ADDRESS_OUT_OF_RANGE;
    }
    int retValue = memory[SP];
    SP += 1;
    if (SP < 0 || SP >= MAX_ADDRESS)
    {
      return VM520_ADDRESS_OUT_OF_RANGE;
    }
    int savedFP = memory[SP];
    SP += 1;
    if (SP < 0 || SP >= MAX_ADDRESS)
    {
      return VM520_ADDRESS_OUT_OF_RANGE;
    }
    int retAddress = memory[SP];
    SP += 1;
    FP = savedFP;
    PC = retAddress;
    if (FP <= 0 || FP > MAX_ADDRESS)
    {
      return VM520_ADDRESS_OUT_OF_RANGE;
    }
    memory[FP - 1] = retValue;
  }
  else
  {
    fprintf(stderr, "procID %d: unexpected opcode (%02x) in format1!\n",
      processorID, opcode);
    exit(-1);
  }
  return 1;
}

// decode and execute a format 2 instruction: call and jmp
//   return 1 to indicate continue execution
//   return VM520_ADDRESS_OUT_OF_RANGE for bad address in call instruction
static int format2(unsigned int processorID, int word, int reg[])
{
  unsigned char opcode = word & 0xFF;
  int storedAddress = word >> 12;
  int effectiveAddress = PC + storedAddress;

  // execute the instruction
  if (opcode == 15) // call
  {
    SP -= 1;
    if (SP < 0 || SP >= MAX_ADDRESS)
    {
      return VM520_ADDRESS_OUT_OF_RANGE;
    }
    memory[SP] = PC;
    PC = effectiveAddress;
    SP -= 1;
    if (SP < 0 || SP >= MAX_ADDRESS)
    {
      return VM520_ADDRESS_OUT_OF_RANGE;
    }
    memory[SP] = FP;
    FP = SP;
    SP -= 1;
    if (SP < 0 || SP >= MAX_ADDRESS)
    {
      return VM520_ADDRESS_OUT_OF_RANGE;
    }
    memory[SP] = 0;
  }
  else if (opcode == 20) // jmp
  {
    PC = effectiveAddress;
  }
  else
  {
    fprintf(stderr, "procID %d: unexpected opcode (%02x) in format2!\n",
      processorID, opcode);
    exit(-1);
  }
  return 1;
}

// decode and execute a format 3 instruction: getpid, getpn, pop, push
//   return 1 to indicate continue execution
//   return VM520_ADDRESS_OUT_OF_RANGE for bad address in pop/push instructions
static int format3(unsigned int processorID, int word, int reg[])
{
  int regNum = (word >> 8) & 0x0F;
  unsigned char opcode = word & 0xFF;

  if (opcode == 22) // getpid
  {
    reg[regNum] = processorID;
  }
  else if (opcode == 23) // getpn
  {
    reg[regNum] = numberOfProcessors;
  }
  else if (opcode == 24) // push
  {
    SP -= 1;
    if (SP < 0 || SP >= MAX_ADDRESS)
    {
      return VM520_ADDRESS_OUT_OF_RANGE;
    }
    memory[SP] = reg[regNum];
  }
  else if (opcode == 25) // pop
  {
    if (SP < 0 || SP >= MAX_ADDRESS)
    {
      return VM520_ADDRESS_OUT_OF_RANGE;
    }
    reg[regNum] = memory[SP];
    SP += 1;
  }
  else
  {
    fprintf(stderr, "procID %d: unexpected opcode (%02x) in format3!\n",
      processorID, opcode);
    exit(-1);
  }
  return 1;
}

// decode and execute a format 4 instruction: ldimm
//   return 1 to indicate continue execution
static int format4(unsigned int processorID, int word, int reg[])
{
  int constant = word >> 12;
  int regNum = (word >> 8) & 0x0F;
  unsigned char opcode = word & 0xFF;

  if (opcode == 3) // ldimm
  {
    reg[regNum] = constant;
  }
  else
  {
    fprintf(stderr, "procID %d: unexpected opcode (%02x) in format4!\n",
      processorID, opcode);
    exit(-1);
  }
  return 1;
}

// decode and execute a format 5 instruction: load, store and ldaddr
//   return 1 to indicate continue execution
//   return VM520_ADDRESS_OUT_OF_RANGE for bad address in load/store
static int format5(unsigned int processorID, int word, int reg[])
{
  int absAddr = PC + (word >> 12);
  int regNum = (word >> 8) & 0x0F;
  unsigned char opcode = word & 0xFF;

  if (opcode == 1) // load
  {
    if (absAddr < 0 || absAddr >= MAX_ADDRESS)
    {
      return VM520_ADDRESS_OUT_OF_RANGE;
    }
    reg[regNum] = memory[absAddr];
  }
  else if (opcode == 2) // store
  {
    if (absAddr < 0 || absAddr >= MAX_ADDRESS)
    {
      return VM520_ADDRESS_OUT_OF_RANGE;
    }
    if (pthread_mutex_lock(&muBus) != 0)
    {
      fprintf(stderr, "procID %d: pthread_mutex_lock failed in store!\n",
        processorID);
      exit(-1);
    }
    memory[absAddr] = reg[regNum];
    if (pthread_mutex_unlock(&muBus) != 0)
    {
      fprintf(stderr, "procID %d: pthread_mutex_unlock failed in store!\n",
        processorID);
      exit(-1);
    }
  }
  else if (opcode == 4) // ldaddr
  {
    reg[regNum] = absAddr;
  }
  else
  {
    fprintf(stderr, "procID %d: unexpected opcode (%02x) in format5!\n",
      processorID, opcode);
    exit(-1);
  }
  return 1;
}

// decode and execute a format 6 instruction
//   addi, subi, muli, divi, addf, subf, mulf, divf
//   return 1 to indicate continue execution
//   return VM520_DIVIDE_BY_ZERO if second operand 0 for divf/divi
static int format6(unsigned int processorID, int word, int reg[])
{
  int regNum1 = (word >> 8) & 0x0F;
  int regNum2 = (word >> 12) & 0x0F;
  unsigned char opcode = word & 0xFF;

  if (opcode == 7) // addf
  {
    float floatResult =
      (*((float *) &reg[regNum1])) + (*((float *) &reg[regNum2]));
    reg[regNum1] = *((int *) &floatResult);
  }
  else if (opcode == 8) // subf
  {
    float floatResult =
      (*((float *) &reg[regNum1])) - (*((float *) &reg[regNum2]));
    reg[regNum1] = *((int *) &floatResult);
  }
  else if (opcode == 9) // divf
  {
    // check for 0.0 or -0.0
    if ((reg[regNum2] == 0) || (reg[regNum2] == 0x80000000))
    {
      return VM520_DIVIDE_BY_ZERO;
    }
    float floatResult =
      (*((float *) &reg[regNum1])) / (*((float *) &reg[regNum2]));
    reg[regNum1] = *((int *) &floatResult);
  }
  else if (opcode == 10) // mulf
  {
    float floatResult =
      (*((float *) &reg[regNum1])) * (*((float *) &reg[regNum2]));
    reg[regNum1] = *((int *) &floatResult);
  }
  else if (opcode == 11) // addi
  {
    reg[regNum1] += reg[regNum2];
  }
  else if (opcode == 12) // subi
  {
    reg[regNum1] -= reg[regNum2];
  }
  else if (opcode == 13) // divi
  {
    if (reg[regNum2] == 0)
    {
      return VM520_DIVIDE_BY_ZERO;
    }
    reg[regNum1] /= reg[regNum2];
  }
  else if (opcode == 14) // muli
  {
    reg[regNum1] *= reg[regNum2];
  }
  else
  {
    fprintf(stderr, "procID %d: unexpected opcode (%02x) in format6!\n",
      processorID, opcode);
    exit(-1);
  }
  return 1;
}

// decode and execute a format 7 instruction: ldind, stind
//   return 1 to indicate continue execution
//   return VM520_ADDRESS_OUT_OF_RANGE for bad address in ldind/stind
static int format7(unsigned int processorID, int word, int reg[])
{
  int regNum1 = (word >> 8) & 0x0F;
  int regNum2 = (word >> 12) & 0x0F;
  int offset = word >> 16;
  unsigned char opcode = word & 0xFF;

  if (opcode == 5) // ldind
  {
    int absAddr = reg[regNum2] + offset;
    if (absAddr < 0 || absAddr >= MAX_ADDRESS)
    {
      return VM520_ADDRESS_OUT_OF_RANGE;
    }
    reg[regNum1] = memory[absAddr];
  }
  else if (opcode == 6) // stind
  {
    int absAddr = reg[regNum2] + offset;
    if (absAddr < 0 || absAddr >= MAX_ADDRESS)
    {
      return VM520_ADDRESS_OUT_OF_RANGE;
    }
    if (pthread_mutex_lock(&muBus) != 0)
    {
      fprintf(stderr, "procID %d: pthread_mutex_lock failed in stind!\n",
        processorID);
      exit(-1);
    }
    memory[absAddr] = reg[regNum1];
    if (pthread_mutex_unlock(&muBus) != 0)
    {
      fprintf(stderr, "procID %d: pthread_mutex_unlock failed in stind!\n",
        processorID);
      exit(-1);
    }
  }
  else
  {
    fprintf(stderr, "procID %d: unexpected opcode (%02x) in format7!\n",
      processorID, opcode);
    exit(-1);
  }
  return 1;
}

// decode and execute a format 8 instruction: blt, bgt, beq, cmpxchg
//   return 1 to indicate continue execution
//   return VM520_ADDRESS_OUT_OF_RANGE for bad address in cmpxchg
static int format8(unsigned int processorID, int word, int reg[])
{
  int regNum1 = (word >> 8) & 0x0F;
  int regNum2 = (word >> 12) & 0x0F;
  int address = word >> 16;
  unsigned char opcode = word & 0xFF;

  if (opcode == 17) // blt
  {
    if (reg[regNum1] < reg[regNum2])
    {
      PC += address;
    }
  }
  else if (opcode == 18) // bgt
  {
    if (reg[regNum1] > reg[regNum2])
    {
      PC += address;
    }
  }
  else if (opcode == 19) // beq
  {
    if (reg[regNum1] == reg[regNum2])
    {
      PC += address;
    }
  }
  else if (opcode == 21) // cmpxchg
  {
    int effectiveAddr = PC + address;
    if (effectiveAddr < 0 || effectiveAddr >= MAX_ADDRESS)
    {
      return VM520_ADDRESS_OUT_OF_RANGE;
    }
    if (pthread_mutex_lock(&muBus) != 0)
    {
      fprintf(stderr, "procID %d: pthread_mutex_lock failed in cmpxchg!\n",
        processorID);
      exit(-1);
    }
    if (reg[regNum1] == memory[effectiveAddr])
    {
      memory[effectiveAddr] = reg[regNum2];
    }
    else
    {
      reg[regNum1] = memory[effectiveAddr];
    }
    if (pthread_mutex_unlock(&muBus) != 0)
    {
      fprintf(stderr, "procID %d: pthread_mutex_unlock failed in cmpxchg!\n",
        processorID);
      exit(-1);
    }
  }
  else
  {
    fprintf(stderr, "procID %d: unexpected opcode (%02x) in format7!\n",
      processorID, opcode);
    exit(-1);
  }
  return 1;
}


