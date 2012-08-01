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

/* 32bit types */
#include <stdint.h>

#include "vm520.h"

#define MEMORY_SIZE 1048576
#define MAX_ADDRESS (MEMORY_SIZE-1)

#define DEBUG_XPVM 1

// forward references
static int readWord(FILE *fp, int *outWord);
static int readSymbol(FILE *fp, char **outSymbol, unsigned int *outAddr);
static int readObjectCode(FILE *fp, int *memory, int length);
static int readBlockXPVM(FILE *fp, int blockNum );
static int getObjLengthXPVM( FILE *, uint32_t, uint64_t *);
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
static int32_t format1XPVM( uint32_t, uint32_t, uint64_t *);
static int32_t format2XPVM( uint32_t, uint32_t, uint64_t *);
static int32_t format3XPVM( uint32_t, uint32_t, uint64_t *);

// linked list to keep track of (symbol,address) pairs for insymbols
static struct insymbol {
  char *symbol;
  unsigned int addr;
  struct insymbol *next;
} *insymbols = NULL;

// the VM memory
static int memory[MEMORY_SIZE];
/*static int64_t memoryXPVM[MEMORY_SIZE];*/

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

#define MAX_REGS 256
#define MAX_NAME_LEN 256
#define BLOCK_REG 254

uint64_t regs[MAX_REGS];

#define MAX_OPCODE_XPVM 150
static struct opcodeInfoXPVM
{
   char* opcode;
   int format;
   int (*formatFunc)(unsigned int procID, uint32_t inst, uint64_t *reg);
}
opcodesXPVM[] =
{
{"0",         0, NULL},        /* 0 */
{"1",         0, NULL},        /* 1 */
{"ldb",       1, format1XPVM}, /* 2 */
{"ldb",       2, format2XPVM}, /* 3 */
{"lds",       1, format1XPVM}, /* 4 */
{"lds",       2, format2XPVM}, /* 5 */
{"ldi",       1, format1XPVM}, /* 6 */
{"ldi",       2, format2XPVM}, /* 7 */
{"ldl",       1, format1XPVM}, /* 8 */
{"ldl",       2, format2XPVM}, /* 9 */
{"ldf",       1, format1XPVM}, /* 10 */
{"ldf",       2, format2XPVM}, /* 11 */
{"ldd",       1, format1XPVM}, /* 12 */
{"ldd",       2, format2XPVM}, /* 13 */
{"ldimm",     3, format3XPVM}, /* 14 */
{"ldimm2",    3, format3XPVM}, /* 15 */
{"stb",       1, format1XPVM}, /* 16 */
{"stb",       2, format2XPVM}, /* 17 */
{"sts",       1, format1XPVM}, /* 18 */
{"sts",       2, format2XPVM}, /* 19 */
{"sti",       1, format1XPVM}, /* 20 */
{"sti",       2, format2XPVM}, /* 21 */
{"stl",       1, format1XPVM}, /* 22 */
{"stl",       2, format2XPVM}, /* 23 */
{"stf",       1, format1XPVM}, /* 24 */
{"stf",       2, format2XPVM}, /* 25 */
{"std",       1, format1XPVM}, /* 26 */
{"std",       2, format2XPVM}, /* 27 */
{"28",        0, NULL},        /* 28 */
{"29",        0, NULL},        /* 29 */
{"30",        0, NULL},        /* 30 */
{"31",        0, NULL},        /* 31 */
{"addl",      1, format1XPVM}, /* 32 */
{"addl",      2, format2XPVM}, /* 33 */
{"subl",      1, format1XPVM}, /* 34 */
{"subl",      2, format2XPVM}, /* 35 */
{"mull",      1, format1XPVM}, /* 36 */
{"mull",      2, format2XPVM}, /* 37 */
{"divl",      1, format1XPVM}, /* 38 */
{"divl",      2, format2XPVM}, /* 39 */
{"reml",      1, format1XPVM}, /* 40 */
{"reml",      2, format2XPVM}, /* 41 */
{"negl",      1, format1XPVM}, /* 42 */
{"addd",      1, format1XPVM}, /* 43 */
{"subd",      1, format1XPVM}, /* 44 */
{"muld",      1, format1XPVM}, /* 45 */
{"divd",      1, format1XPVM}, /* 46 */
{"negd",      1, format2XPVM}, /* 47 */
{"cvtld",     1, format1XPVM}, /* 48 */
{"cvtdl",     1, format1XPVM}, /* 49 */
{"lshift",    1, format1XPVM}, /* 50 */
{"lshift",    2, format2XPVM}, /* 51 */
{"rshift",    1, format1XPVM}, /* 52 */
{"rshift",    2, format2XPVM}, /* 53 */
{"rshiftu",   1, format1XPVM}, /* 54 */
{"rshiftu",   2, format2XPVM}, /* 55 */
{"and",       1, format1XPVM}, /* 56 */
{"or",        1, format1XPVM}, /* 57 */
{"xor",       1, format1XPVM}, /* 58 */
{"ornot",     1, format1XPVM}, /* 59 */
{"60",        0, NULL},        /* 60 */
{"61",        0, NULL},        /* 61 */
{"62",        0, NULL},        /* 62 */
{"63",        0, NULL},        /* 63 */
{"cmpeq",     1, format1XPVM}, /* 64 */
{"cmpeq",     2, format2XPVM}, /* 65 */
{"cmple",     1, format1XPVM}, /* 66 */
{"cmple",     2, format2XPVM}, /* 67 */
{"cmplt",     1, format1XPVM}, /* 68 */
{"cmplt",     2, format2XPVM}, /* 69 */
{"cmpule",    1, format1XPVM}, /* 70 */
{"cmpule",    2, format2XPVM}, /* 71 */
{"cmpult",    1, format1XPVM}, /* 72 */
{"cmpult",    2, format2XPVM}, /* 73 */
{"fcmpeq",    1, format1XPVM}, /* 74 */
{"fcmple",    1, format1XPVM}, /* 75 */
{"fcmplt",    1, format1XPVM}, /* 76 */
{"77",        0, NULL},        /* 77 */
{"78",        0, NULL},        /* 78 */
{"79",        0, NULL},        /* 79 */
{"jmp",       3, format3XPVM}, /* 80 */
{"jmp",       1, format1XPVM}, /* 81 */
{"btrue",     3, format3XPVM}, /* 82 */
{"bfalse",    3, format3XPVM}, /* 83 */
{"84",        0, NULL},        /* 84 */
{"85",        0, NULL},        /* 85 */
{"86",        0, NULL},        /* 86 */
{"87",        0, NULL},        /* 87 */
{"88",        0, NULL},        /* 88 */
{"89",        0, NULL},        /* 89 */
{"90",        0, NULL},        /* 90 */
{"91",        0, NULL},        /* 91 */
{"92",        0, NULL},        /* 92 */
{"93",        0, NULL},        /* 93 */
{"94",        0, NULL},        /* 94 */
{"95",        0, NULL},        /* 95 */
{"malloc",    1, format1XPVM}, /* 96 */
{"mmexa",     1, format1XPVM}, /* 97 */
{"mmexa",     2, format2XPVM}, /* 98 */
{"atraits",   1, format1XPVM}, /* 99 */
{"dtraits",   1, format1XPVM}, /* 100 */
{"rannots",   1, format1XPVM}, /* 101 */
{"towner",    1, format1XPVM}, /* 102 */
{"lock",      1, format1XPVM}, /* 103 */
{"unlock",    1, format1XPVM}, /* 104 */
{"wait",      1, format1XPVM}, /* 105 */
{"sig",       1, format1XPVM}, /* 106 */
{"sigall",    1, format1XPVM}, /* 107 */
{"108",       0, NULL},        /* 108 */
{"109",       0, NULL},        /* 109 */
{"110",       0, NULL},        /* 110 */
{"111",       0, NULL},        /* 111 */
{"ldfunc",    3, format3XPVM}, /* 112 */
{"ldfunc",    1, format1XPVM}, /* 113 */
{"call",      2, format2XPVM}, /* 114 */
{"calln",     2, format2XPVM}, /* 115 */
{"ret",       1, format1XPVM}, /* 116 */
{"117",       0, NULL},        /* 117 */
{"118",       0, NULL},        /* 118 */
{"119",       0, NULL},        /* 119 */
{"120",       0, NULL},        /* 120 */
{"121",       0, NULL},        /* 121 */
{"122",       0, NULL},        /* 122 */
{"123",       0, NULL},        /* 123 */
{"124",       0, NULL},        /* 124 */
{"125",       0, NULL},        /* 125 */
{"126",       0, NULL},        /* 126 */
{"127",       0, NULL},        /* 127 */
{"throw",     1, format1XPVM}, /* 128 */
{"retrieve",  1, format1XPVM}, /* 129 */
{"130",       0, NULL},        /* 130 */
{"131",       0, NULL},        /* 131 */
{"132",       0, NULL},        /* 132 */
{"133",       0, NULL},        /* 133 */
{"134",       0, NULL},        /* 134 */
{"135",       0, NULL},        /* 135 */
{"136",       0, NULL},        /* 136 */
{"137",       0, NULL},        /* 137 */
{"138",       0, NULL},        /* 138 */
{"139",       0, NULL},        /* 139 */
{"140",       0, NULL},        /* 140 */
{"141",       0, NULL},        /* 141 */
{"142",       0, NULL},        /* 142 */
{"143",       0, NULL},        /* 143 */
{"initProc",  2, format2XPVM}, /* 144 */
{"join",      1, format1XPVM}, /* 145 */
{"join2",     1, format1XPVM}, /* 146 */
{"whoami",    1, format1XPVM}, /* 147 */
};

struct _block
{
  uint32_t      length;
  uint32_t      frame_size;
  uint32_t      num_except_handlers;
  uint32_t      num_outsymbol_refs;
  uint32_t      length_aux_data;
  uint64_t      annots;
  char          name[256];
  char          *data;
  char          *aux_data;
  /*struct _block *next;*/
} typedef block;

uint32_t reverseEndianness( uint32_t toRev )
{
  uint32_t reversed = 0;
  uint32_t c1 = toRev & 0xFF000000;
  uint32_t c2 = toRev & 0x00FF0000;
  uint32_t c3 = toRev & 0x0000FF00;
  uint32_t c4 = toRev & 0x000000FF;

  /*fprintf( stderr, "c1: %8x\n", c1 >> 24);
  fprintf( stderr, "c2: %8x\n", c2 >> 8);
  fprintf( stderr, "c3: %8x\n", c3 << 8);
  fprintf( stderr, "c4: %8x\n", c4 << 24);*/

  reversed = ((c1 >> 24) | (c2 >> 8) | (c3 << 8) | c4 << 24);
  return reversed;
}

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
int32_t loadObjectFileXPVM( char *filename, int32_t *errorNumber )
{
  FILE *fp;
  uint32_t blockCount = 0;
  uint64_t objLength = 0;
  const uint32_t MAGIC = 0x31303636;
  uint32_t magic = 0;
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
#if DEBUG_XPVM
  fprintf( stderr, "magic: %x\n", magic );
  fprintf( stderr, "magic: %x\n", reverseEndianness(magic) );
#endif
  magic = reverseEndianness(magic);
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
  blockCount = reverseEndianness(blockCount);

  if(!(regs[BLOCK_REG] = calloc( blockCount, sizeof(block) )))
  {
    fprintf( stderr, "Error: malloc failed in loadObjectFileXPVM");
    exit(-1);
  }

#if DEBUG_XPVM
  fprintf(stderr, "blockCount: %d\n", blockCount );
#endif

  /* 
   * Get the length of the object file 
   * and do a sanity check on the format 
   */
  if( !getObjLengthXPVM( fp, blockCount, &objLength ) )
  {
    *errorNumber = -3;
    return 0;
  }

  /* Read the blocks into memory */
  for( i = 0; i < blockCount; i++ )
  {
    if (!readBlockXPVM(fp, i))
    {
      *errorNumber = -3;
      return 0;
    }
  }

  return 1;
}

/*
 * getObjLengthXPVM
 *
 * Gets the length of the object file in bytes.
 * Takes a file pointer, a 32 bit block count and a
 * pointer to a 64 bit int for returning the length in
 * bytes.
 * Returns 1 on success, 0 if the object file is malformed
 * (The number of blocks is not what was indicated in the file header).
 * FIXME: Should this function also check to make sure there are no
 * out symbol references?
 * Used for the XPVM.
 */
static int getObjLengthXPVM( FILE *fp, uint32_t blockCount, uint64_t *objLength )
{
  fpos_t *fp_pos = NULL;malloc( sizeof(fpos_t) );
  uint32_t cur_length = 0;
  int i = 0;

  if(!( fp_pos = malloc(sizeof(fpos_t))))
  {
    fprintf( stderr, "Error: malloc failed in getObjLength\n");
    exit(-1);
  }

  /* Save the current position in the file. */
  fprintf( stderr, "In getObjLength\n");
  if( 0 > fgetpos( fp, fp_pos ) )
    return 0;

  fprintf( stderr, "After intial read\n");

  *objLength = 0;
  for( i = 0; i < blockCount; i++ )
  {
    if( !readWord( fp, (int*) &cur_length ) )
      return 0;
    cur_length = reverseEndianness(cur_length);
    if( 0 > fseek( fp, cur_length-1, SEEK_CUR ) )
      return 0;
    *objLength += cur_length;
  }

  fprintf( stderr, "After objLength calculation\n");

  /* Return the file reading to its original position */
  if( 0 > fsetpos( fp, fp_pos ) )
    return 0;

  free( fp_pos );

  fprintf( stderr, "Leaving getObjLength\n");

  return 1;
}

/*
 * readBlockXPVM
 *
 * Takes a file pointer to the object code
 * and the number of the block which needs to
 * be read in. Reads in all the information
 * and stores it into the already allocated
 * array of blocks stored at register BLOCK_REG.
 */
static int readBlockXPVM( FILE *fp, int blockNum )
{
  block b = ((block*)regs[BLOCK_REG])[blockNum];
  int i = 0;
  int64_t temp = 0;
  /* Read name string from block */
  while( (b.name[i] = fgetc( fp )) && ++i < MAX_NAME_LEN );
  /* Name too long */
  if( MAX_NAME_LEN == i )
    return 0;

#if DEBUG_XPVM
  fprintf( stderr, "name: %s\n", b.name );
#endif

  /* Read trait annotations */
  b.annots = 0;
  for( i = 0; i < 8; i++ )
  {
    temp = (int64_t)fgetc( fp );
    fprintf( stderr, "%d: %08x\n", i, temp );
    fprintf( stderr, "shift: %d\n", (64 - (i+1)*8) );
    fprintf( stderr, "%ds: %08x\n", i, (temp << (64 - (i+1)*8) ) );
    b.annots |=  ( temp << (64 - (i+1)*8) );
    fprintf( stderr, "annots: %08x\n", b.annots );
  }

#if DEBUG_XPVM
  fprintf( stderr, "annots: %08x\n", b.annots );
#endif

  return 1;  
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

/*
 * fetchExecuteXPVM
 *
 * XPVM version of the fetch/execute function
 *
 */
/* 256 registers per processor */
#define NUM_REGS 256
static void *fetchExecuteXPVM(void *v)
{
  int i;

  // the processor ID is passed in
  int processorID = (int) v;

  // the registers
  // int reg[16];
  uint64_t reg[NUM_REGS];
  for (i = 0; i < NUM_REGS; i += 1)
    reg[i] = 0;
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
      /*
      fprintf(stderr, "<%d>: %08x %08x %08x %08x %08x %08x %08x %08x\n",
        processorID,
        reg[0], reg[1], reg[2], reg[3], reg[4], reg[5], reg[6], reg[7]);
      fprintf(stderr, "<%d>: %08x %08x %08x %08x %08x %08x %08x %08x\n",
        processorID,
        reg[8], reg[9], reg[10], reg[11], reg[12], reg[13], reg[14], reg[15]);
      */

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
    //int word = memory[PC];
    uint32_t word = memory[PC];

    // update PC
    PC += 1;

    // execute
    //unsigned char opcode = word & 0xFF;
    uint32_t opcode = (word & 0xFF000000) >> 24;
    if (opcode > MAX_OPCODE) // illegal instruction
    {
      return  (void *) VM520_ILLEGAL_INSTRUCTION;
    }
    int32_t ret = opcodesXPVM[opcode].formatFunc(processorID, word, reg);
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

/************* format functions *****************
 *
 * The format functions for the XPVM take three arguments each.
 * The first argument sepcifies the processor ID, the second
 * are the 32 bits that specify the instruction, this will need
 * to be taken apart by the format function and how this will be
 * done shall depend on the format of which there are three.
 * The last argument will be an array of 64 bit registers.
 *
 */

static int32_t format1XPVM( uint32_t pID, uint32_t inst, uint64_t *reg )
{
  uint8_t opcode = (inst & 0xFF000000) >> 24;
  uint8_t ri     = (inst & 0x00FF0000) >> 16;
  uint8_t rj     =  (inst & 0x0000FF00) >> 8;
  uint8_t rk     = (inst & 0x000000FF);
  int64_t abs_addr = 0;

  switch( opcode )
  {
    case 0x02: /* ldb */
    case 0x04: /* lds */
    case 0x06: /* ldi */
    case 0x08: /* ldl */
    case 0x0A: /* ldf */
    case 0x0C: /* ldd */
    /*
      abs_addr = reg[rj]+reg[rk];
      if( abs_addr < 0 || abs_addr >= MAX_ADDRESS )
        return VM520_ADDRESS_OUT_OF_RANGE;
      reg[ri] = memoryXPVM[ abs_addr ];
      break;*/
    case 0x10: /* sdb */
    case 0x12: /* sds */
    case 0x14: /* sdi */
    case 0x16: /* sdl */
    case 0x18: /* sdf */
    case 0x1A: /* sdd */
    /*
      abs_addr = reg[rj]+reg[rk];
      if( abs_addr < 0 || abs_addr >= MAX_ADDRESS )
        return VM520_ADDRESS_OUT_OF_RANGE;
      memoryXPVM[ abs_addr ] = reg[ ri ];
      break;*/
    case 0x20: /* addl */
      reg[ri] = (long)reg[rj] + (long)reg[rk];
      break;
    case 0x22: /* subl */
      reg[ri] = (long)reg[rj] - (long)reg[rk];
      break;
    case 0x24: /* mull */
      reg[ri] = (long)reg[rj] * (long)reg[rk];
      break;
    case 0x26: /* divl */
      reg[ri] = (long)reg[rj] / (long)reg[rk];
      break;
    case 0x28: /* reml */
      reg[ri] = (long)reg[rj] % (long)reg[rk];
      break;
    case 0x2A: /* negl */
      reg[ri] = -(long)reg[rj];
      break;
    case 0x2B: /* addd */
      reg[ri] = (double)reg[rj] + (double)reg[rk];
      break;
    case 0x2C: /* subd */
      reg[ri] = (double)reg[rj] - (double)reg[rk];
      break;
    case 0x2D: /* muld */
      reg[ri] = (double)reg[rj] * (double)reg[rk];
      break;
    case 0x2E: /* divd */
      reg[ri] = (double)reg[rj] / (double)reg[rk];
      break;
    case 0x2F: /* negd */
      reg[ri] = -(double)reg[rj];
      break;
    case 0x30: /* cvtld */
      reg[ri] = (double)reg[rj];
      break;
    case 0x31: /* cvtdl */
      reg[ri] = (long)reg[rj];
      break;
    default: /* stuff breaks */
      fprintf( stderr, "Bad opcode, exiting!\n");
      exit(-1);
      break;
  }
  return 1;
}

static int32_t format2XPVM( uint32_t pID, uint32_t inst, uint64_t *reg )
{
  uint8_t opcode = (inst & 0xFF000000) >> 24;
  uint8_t regi   = (inst & 0x00FF0000) >> 16;
  uint8_t regj   = (inst & 0x0000FF00) >> 8;
  uint8_t const8 = (inst & 0x000000FF);
  return 0;
}

static int32_t format3XPVM( uint32_t pID, uint32_t inst, uint64_t *reg )
{
  uint8_t  opcode  = (inst & 0xFF000000) >> 24;
  uint8_t  regi    = (inst & 0x00FF0000) >> 16;
  uint16_t const16 = (inst & 0x0000FFFF);
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


