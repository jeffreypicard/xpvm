/*
 * xpvm.c
 *
 * Implementation of the XPVM
 *
 * Author: Jeffrey Picard
 * Heavily based on Professor Hatcher's vm520.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

/* 32-bit and 64-bit types */
#include <stdint.h>

#include "xpvm.h"

#define MEMORY_SIZE 1048576
#define MAX_ADDRESS (MEMORY_SIZE-1)

#define DEBUG_XPVM 1

#define EXIT_WITH_ERROR(...){     \
  fprintf( stderr, __VA_ARGS__ ); \
  exit( -1 );                     \
}                                 \

#define CAST_INT (uint32_t)

/* forward references */
static int readWord(FILE *fp, int *outWord);
static int readBlockXPVM(FILE *fp, int blockNum );
static int getObjLengthXPVM( FILE *, uint32_t, uint64_t *);
static void *fetchExecuteXPVM(void *v);
typedef struct _stackNode stackNode;
static int32_t format1XPVM( uint32_t, uint32_t, uint64_t*, stackNode*);
static int32_t format2XPVM( uint32_t, uint32_t, uint64_t*, stackNode*);
static int32_t format3XPVM( uint32_t, uint32_t, uint64_t*, stackNode*);

/* linked list to keep track of (symbol,address) pairs for insymbols
static struct insymbol {
  char *symbol;
  unsigned int addr;
  struct insymbol *next;
} *insymbols = NULL;*/

#define MAX_REGS 256
#define MAX_NAME_LEN 256
#define BLOCK_REG 254

#define PCX regs2[253]

uint64_t regs[MAX_REGS];
static uint32_t blockCount = 0;
static unsigned char *pcXPVM = 0;
uint64_t blockPtr = 0;

struct _stackFrame
{
  uint64_t      pc;
  uint64_t      reg255;
  uint64_t      retReg;
  unsigned char *locals;
} typedef stackFrame;

/*
 * Struct for the VM stack.
 */
struct _stackNode
{
  stackFrame        *data;
  struct _stackNode  *prev;
} typedef stackNode;

#define MAX_OPCODE_XPVM 150
static struct opcodeInfoXPVM
{
   char* opcode;
   int format;
   int (*formatFunc)(unsigned int procID, uint32_t inst, uint64_t *reg,
                     stackNode *stack );
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
  unsigned char *data;
  unsigned char *aux_data;
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

/*
 * cleanup
 *
 * This function is passed to atexit in the loadObjectFileXPVM
 * function. It cleans up all the memory used when loading the
 * object file.
 */
void cleanup( void )
{
  int i = 0;
  if( blockCount )
  {
    if( blockPtr )
    {
      for( i = 0; i < blockCount; i++ )
      {
        fprintf( stderr, "Freeing stuff inside block.\n");
        //block *b = ((block**) CAST_INT regs[BLOCK_REG])[i];
        block *b = ((block**) CAST_INT blockPtr)[i];
        if( b )
        {
          fprintf( stderr, "b->data: %p\n", b->data );
          fprintf( stderr, "b->aux_data: %p\n", b->aux_data );
          free( b->data );
          free( b->aux_data );
          free( b );
        }
      }
      free( (block**) CAST_INT regs[BLOCK_REG] );
    }
  }
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
  uint64_t objLength = 0;
  const uint32_t MAGIC = 0x31303636;
  uint32_t magic = 0;
  int i = 0;
  atexit( cleanup );

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

  if(!(blockPtr = (uint64_t) CAST_INT calloc( blockCount, sizeof(block*) )))
    EXIT_WITH_ERROR("Error: malloc failed in loadObjectFileXPVM");

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

  fclose( fp );

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
  fpos_t *fp_pos = NULL;
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
#if DEBUG_XPVM
  fprintf( stderr, "------- Reading block %d from object file. -------\n", 
                   blockNum );
#endif

  block *b = calloc( 1, sizeof(block) );
  if( !b )
    EXIT_WITH_ERROR("Error: malloc failed in readBlockXPVM");
  ((block**) CAST_INT blockPtr)[blockNum] = b;
  int i = 0;
  /*uint64_t temp = 0;*/
  /* Read name string from block */
  while( (b->name[i] = fgetc( fp )) && ++i < MAX_NAME_LEN );
  /* Name too long */
  if( MAX_NAME_LEN == i )
    return 0;

#if DEBUG_XPVM
  fprintf( stderr, "name: %s\n", b->name );
#endif

  /* Read trait annotations */
  for( i = 0; i < 8; i++ )
    b->annots |=  ( (uint64_t)fgetc( fp ) << (64 - (i+1)*8) );

#if DEBUG_XPVM
  fprintf( stderr, "annots: %016llx\n", b->annots );
#endif

  /* Read frame size */
  for( i = 0; i < 4; i++ )
    b->frame_size |=  ( (uint32_t)fgetc( fp ) << (32 - (i+1)*8) );

#if DEBUG_XPVM
  fprintf( stderr, "frame_size: %08x\n", b->frame_size );
#endif

  /* Read contents length */
  for( i = 0; i < 4; i++ )
    b->length |= ( (uint32_t)fgetc( fp ) << (32 - (i+1)*8) );

#if DEBUG_XPVM
  fprintf( stderr, "length: %08x\n", b->length );
#endif

  /* Allocate contents */
  b->data = calloc( b->length, sizeof(char) );
  if( !b->data && b->length )
  {
    fprintf( stderr, "Error: malloc failed in readBlockXPVM\n");
    exit(-1);
  }

  /* Read contents */
  for( i = 0; i < b->length; i++ )
  {
    b->data[i] = fgetc( fp );
#if DEBUG_XPVM
    fprintf( stderr, "%02x\n", b->data[i] );
#endif
  }

  /* Read number of exception handlers */
  for( i = 0; i < 4; i++ )
    b->num_except_handlers |= ( (uint32_t)fgetc( fp ) << (32 - (i+1)*8) );

#if DEBUG_XPVM
  fprintf( stderr, "num_except_handlers: %08x\n", b->num_except_handlers );
#endif

  /* Read number of outsymbol references */
  for( i = 0; i < 4; i++ )
    b->num_outsymbol_refs |= ( (uint32_t)fgetc( fp ) << (32 - (i+1)*8) );

#if DEBUG_XPVM
  fprintf( stderr, "num_outsymbol_refs: %08x\n", b->num_outsymbol_refs );
#endif

  /* Read length of auxiliary data */
  for( i = 0; i < 4; i++ )
    b->length_aux_data |= ( (uint32_t)fgetc( fp ) << (32 - (i+1)*8) );

#if DEBUG_XPVM
  fprintf( stderr, "length_aux_data: %08x\n", b->length_aux_data );
#endif

  /* Allocate auxiliary data */
  b->aux_data = calloc( b->length_aux_data, sizeof(char) );
  if( !b->aux_data && b->length_aux_data )
  {
    fprintf( stderr, "Error: malloc failed in readBlockXPVM\n");
    exit(-1);
  }

  /* Read auxiliary data */
  for( i = 0; i < b->length_aux_data; i++ )
  {
    b->aux_data[i] = fgetc( fp );
#if DEBUG_XPVM
    fprintf( stderr, "%02x\n", b->aux_data[i] );
#endif
  }

#if DEBUG_XPVM
  fprintf( stderr, "------- Block %d successfully "
                   "read from object file. -------\n", 
                   blockNum );
#endif

  return 1;  
}

struct _cmdArg
{
  char s[0];
} typedef cmdArg;

struct _feArg
{
  uint64_t *regBank;
  uint64_t work;
  int argc;
} typedef feArg;

/*
 * doInitProc
 *
 * C function implementation of the initProc
 * opcode. also used to start the VM executing
 * the main function.
 */
int doInitProc( int64_t *procID, uint64_t work, int argc, 
                 uint64_t *regBank )
{
  pthread_t *pt = calloc( 1, sizeof(pthread_t) );

#if DEBUG_XPVM
  fprintf( stderr, "Starting processors.\n");
#endif

  feArg *ar3 = calloc( 1, sizeof(feArg) );
  if( !ar3 )
    EXIT_WITH_ERROR("Error: malloc failed in doInitProc\n");
  ar3->regBank = regBank;
  ar3->work = work;
  ar3->argc = argc;

  if (pthread_create(pt, NULL, fetchExecuteXPVM, (void *) ar3) != 0)
  {
    perror("error in thread create");
    exit(-1);
  }

  *procID = (uint64_t) CAST_INT pt;

  return 0;
}

int doProcJoin( uint64_t procID, uint64_t *retVal )
{
  pthread_t *pt = (pthread_t*) CAST_INT procID;
  void *ret;
  if (pthread_join(*pt, &ret) != 0)
  {
    perror("error in thread join");
    exit(-1);
  }
  *retVal = (uint64_t) CAST_INT ret;

  return 0;
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

#define PC reg[15]
#define SP reg[14]
#define FP reg[13]

uint32_t assembleInst( unsigned char *pc )
{
  uint8_t c1 = pc[0];
  uint8_t c2 = pc[1];
  uint8_t c3 = pc[2];
  uint8_t c4 = pc[3];
  fprintf( stderr, "\tc1: %02x\n"
                   "\tc2: %02x\n"
                   "\tc3: %02x\n"
                   "\tc4: %02x\n",
                   c1,c2,c3,c4);
  uint32_t inst = (c1 << 24) | (c2 << 16) | (c3 << 8) | c4;
  return inst;
}

struct _retStruct
{
  int status;
  int64_t retVal;
} typedef retStruct;

/*
 * fetchExecuteXPVM
 *
 * XPVM version of the fetch/execute function
 *
 */
static void *fetchExecuteXPVM(void *v)
{
#if DEBUG_XPVM
  fprintf( stderr, "In fetchExecuteXPVM.\n");
#endif
  /*int i = 0;*/
  feArg *args = (feArg*)v;
  uint64_t *regBank = args->regBank;
  int argc = args->argc;
  uint64_t work = args->work;

  free( args );

  int i = 0;
  int len = 0;
  cmdArg *ar1 = NULL, *ar2 = NULL;
  /* Inialize the VM to run */
  uint64_t regs2[256];
  regs2[BLOCK_REG] = blockPtr; /* FIXME */
  stackNode *stack2 = calloc( 1, sizeof(stackNode) );
  if( !stack2 )
    EXIT_WITH_ERROR("Error: malloc failed in loadObjectFileXPVM");
  stack2->data = calloc( 1, sizeof(stackFrame) );
  if( !stack2->data )
    EXIT_WITH_ERROR("Error: malloc failed in loadObjectFileXPVM");
  block *b = ((block**) CAST_INT regs2[BLOCK_REG])[0];
  if( 0 < b->frame_size )
    stack2->data->locals = calloc( b->frame_size, sizeof(char) );
  
  /* If work is null, set to pc to the main function in
   * the object file. Otherwise set it to work 
   */
  if( !work )
    PCX = (uint64_t) CAST_INT b->data;
  else
    PCX = work;

  /* FIXME */
  /* This currently only supports 10 args */
  for( i = 0; i < argc && i < 10; i++ )
  {
    ar1 = ((cmdArg*) CAST_INT (regBank+i));
    len = strlen( ar1->s );
    ar2 = calloc( 1, sizeof(cmdArg) + len + 1 );
    if( !ar2 )
      EXIT_WITH_ERROR("Error: malloc failed in doInitProc\n");
    strcpy( ar2->s, ar1->s );
    regs2[i] = (uint64_t) CAST_INT ar2;
  }

  /*stackNode *stack = args->stack;
  uint64_t *regs2 = args->regs;*/

  /* the processor ID is passed in */
  /*int processorID = args->procNum;*/
  retStruct *r = calloc( 1, sizeof(retStruct) );
  if( !r )
    EXIT_WITH_ERROR("Error: malloc failed in doInitProc\n");

  /* fetch/execute cycle */
  while (1)
  {
#if DEBUG_XPVM
    fprintf( stderr, "------- start fetch/execute cycle -------\n");
#endif
    /* check to see if the PC is in range
    if ((PC < 0) || (PC >= MAX_ADDRESS))
    {
      return (void *) VM520_ADDRESS_OUT_OF_RANGE;
    }*/

    // fetch
    //int word = memory[PC];
    uint32_t word = assembleInst( (unsigned char*) CAST_INT PCX );
#if DEBUG_XPVM
    fprintf( stderr, "\tword: %08x\n", word );
#endif

    // update PC
    PCX += 4;

    // execute
    //unsigned char opcode = word & 0xFF;
    uint32_t opcode = (word & 0xFF000000) >> 24;
#if DEBUG_XPVM
    fprintf( stderr, "\topcode: %d\n", opcode );
#endif
    if (opcode > MAX_OPCODE_XPVM) // illegal instruction
    {
      return  (void *) VM520_ILLEGAL_INSTRUCTION;
    }
    int32_t ret = opcodesXPVM[opcode].formatFunc(1, word, regs2, stack2 );
#if DEBUG_XPVM
    fprintf( stderr, "\tret: %d\n", ret );
#endif
    if (ret <= 0)
    {
      /* Check if ret was called */
      if( 0x74 == opcode )
      {
        r->retVal = regs2[stack2->data->retReg];
        stackNode *oldNode = stack2;
        stack2 = stack2->prev;
        free( oldNode->data );
        free( oldNode );
      }
      r->status = ret;
      pthread_exit( (void*) CAST_INT r );
      return (void *) ret;
    }
    else if (ret != 1)
    {
      fprintf(stderr,
        "Unexpected return value from formatFunc!\n");
      exit(-1);
    }
#if DEBUG_XPVM
    fprintf( stderr, "------- end fetch/execute cycle -------\n");
#endif
  }
  
  /* won't reach here */
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

static int32_t format1XPVM( uint32_t pID, uint32_t inst, uint64_t *reg,
                            stackNode *stack )
{
  uint8_t opcode = (inst & 0xFF000000) >> 24;
  uint8_t ri     = (inst & 0x00FF0000) >> 16;
  uint8_t rj     = (inst & 0x0000FF00) >> 8;
  uint8_t rk     = (inst & 0x000000FF);
  /*int64_t abs_addr = 0;*/

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
    case 0x74: /* ret */
      fprintf( stderr, "\treg: %p\n", reg );
      fprintf( stderr, "\tstack: %p\n", stack );
      fprintf( stderr, "\tr1: %lld\n", reg[1] );
      reg[stack->data->retReg] = reg[rj];
      pcXPVM = (unsigned char*) CAST_INT stack->data->pc;
      /* pop frame */
      fprintf( stderr, "\tr1: %lld\n", reg[1] );
      /* popped last stack, halt */
      if( !stack->prev )
        return 0;
      stackNode *oldNode = stack;
      stack = stack->prev;
      free( oldNode->data );
      free( oldNode );
      break;
    default: /* stuff breaks */
      fprintf( stderr, "Bad opcode, exiting!\n");
      exit(-1);
      break;
  }
  return 1;
}

static int32_t format2XPVM( uint32_t pID, uint32_t inst, uint64_t *reg,
                            stackNode *stack )
{
  /*
  uint8_t opcode = (inst & 0xFF000000) >> 24;
  uint8_t ri     = (inst & 0x00FF0000) >> 16;
  uint8_t rj     = (inst & 0x0000FF00) >> 8;
  int8_t const8  = (inst & 0x000000FF);
  */
  return 0;
}

static int32_t format3XPVM( uint32_t pID, uint32_t inst, uint64_t *reg,
                            stackNode *stack )
{
  uint8_t opcode   = (inst & 0xFF000000) >> 24;
  uint8_t ri       = (inst & 0x00FF0000) >> 16;
  int16_t const16  = (inst & 0x0000FFFF);

  switch( opcode )
  {
    case 0x0E: /* ldimm */
      reg[ri] = (int64_t)const16;
      break;
    default: /* stuff breaks */
      EXIT_WITH_ERROR("Bad opcode, exiting!");
      break;
  }

  return 1;
}
