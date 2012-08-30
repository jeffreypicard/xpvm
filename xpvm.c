/*
 * xpvm.c
 *
 * Implementation of the XPVM.
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

/* forward references */
static int readWordXPVM(FILE *fp, uint32_t *outWord);
static int readBlockXPVM(FILE *fp, int blockNum );
static int getObjLengthXPVM( FILE *, uint32_t, uint64_t *);
static void *fetchExecuteXPVM(void *v);

/* linked list to keep track of (symbol,address) pairs for insymbols
static struct insymbol {
  char *symbol;
  unsigned int addr;
  struct insymbol *next;
} *insymbols = NULL;*/

uint64_t regs[MAX_REGS];
static uint32_t blockCount = 0;
uint64_t blockPtr = 0;
uint64_t *blockPtr2 = 0;

/* 
 * Table of opcodes.
 *    Indexed by opcode number.
 *    Contains opcode name as a string, format type 
 *    as an int and a function pointer to the C 
 *    implementation of the opcode.
 */
#define MAX_OPCODE_XPVM 150
static struct opcodeInfoXPVM
{
  char* opcode;
  int format;
  int (*formatFunc)( unsigned int proc_id, uint64_t *reg, stackFrame **stack,
                     uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 );
}
opcodesXPVM[] =
{
{"0",         0, NULL},        /* 0 */
{"1",         0, NULL},        /* 1 */
{"ldb",       1, ldb_2},       /* 2 */
{"ldb",       2, ldb_3},       /* 3 */
{"lds",       1, lds_4},       /* 4 */
{"lds",       2, lds_5},       /* 5 */
{"ldi",       1, ldi_6},       /* 6 */
{"ldi",       2, ldi_7},       /* 7 */
{"ldl",       1, ldl_8},       /* 8 */
{"ldl",       2, ldl_9},       /* 9 */
{"ldf",       1, NULL}, /* 10 */
{"ldf",       2, NULL}, /* 11 */
{"ldd",       1, NULL}, /* 12 */
{"ldd",       2, NULL}, /* 13 */
{"ldimm",     3, ldimm_14},    /* 14 */
{"ldimm2",    3, ldimm2_15},   /* 15 */
{"stb",       1, NULL}, /* 16 */
{"stb",       2, stb_17}, /* 17 */
{"sts",       1, NULL}, /* 18 */
{"sts",       2, NULL}, /* 19 */
{"sti",       1, NULL}, /* 20 */
{"sti",       2, sti_21},      /* 21 */
{"stl",       1, NULL}, /* 22 */
{"stl",       2, NULL}, /* 23 */
{"stf",       1, NULL}, /* 24 */
{"stf",       2, NULL}, /* 25 */
{"std",       1, NULL}, /* 26 */
{"std",       2, NULL}, /* 27 */
{"28",        0, NULL},        /* 28 */
{"29",        0, NULL},        /* 29 */
{"30",        0, NULL},        /* 30 */
{"31",        0, NULL},        /* 31 */
{"addl",      1, addl_32},     /* 32 */
{"addl",      2, NULL}, /* 33 */
{"subl",      1, subl_34},     /* 34 */
{"subl",      2, NULL}, /* 35 */
{"mull",      1, mull_36},     /* 36 */
{"mull",      2, NULL}, /* 37 */
{"divl",      1, NULL}, /* 38 */
{"divl",      2, NULL}, /* 39 */
{"reml",      1, NULL}, /* 40 */
{"reml",      2, NULL}, /* 41 */
{"negl",      1, NULL}, /* 42 */
{"addd",      1, NULL}, /* 43 */
{"subd",      1, NULL}, /* 44 */
{"muld",      1, NULL}, /* 45 */
{"divd",      1, NULL}, /* 46 */
{"negd",      1, NULL}, /* 47 */
{"cvtld",     1, NULL}, /* 48 */
{"cvtdl",     1, NULL}, /* 49 */
{"lshift",    1, NULL}, /* 50 */
{"lshift",    2, NULL}, /* 51 */
{"rshift",    1, NULL}, /* 52 */
{"rshift",    2, NULL}, /* 53 */
{"rshiftu",   1, NULL}, /* 54 */
{"rshiftu",   2, NULL}, /* 55 */
{"and",       1, NULL}, /* 56 */
{"or",        1, NULL}, /* 57 */
{"xor",       1, NULL}, /* 58 */
{"ornot",     1, NULL}, /* 59 */
{"60",        0, NULL},        /* 60 */
{"61",        0, NULL},        /* 61 */
{"62",        0, NULL},        /* 62 */
{"63",        0, NULL},        /* 63 */
{"cmpeq",     1, NULL}, /* 64 */
{"cmpeq",     2, NULL}, /* 65 */
{"cmple",     1, NULL}, /* 66 */
{"cmple",     2, NULL}, /* 67 */
{"cmplt",     1, NULL}, /* 68 */
{"cmplt",     2, NULL}, /* 69 */
{"cmpule",    1, NULL}, /* 70 */
{"cmpule",    2, NULL}, /* 71 */
{"cmpult",    1, NULL}, /* 72 */
{"cmpult",    2, NULL}, /* 73 */
{"fcmpeq",    1, NULL}, /* 74 */
{"fcmple",    1, NULL}, /* 75 */
{"fcmplt",    1, NULL}, /* 76 */
{"77",        0, NULL},        /* 77 */
{"78",        0, NULL},        /* 78 */
{"79",        0, NULL},        /* 79 */
{"jmp",       3, NULL}, /* 80 */
{"jmp",       1, NULL}, /* 81 */
{"btrue",     3, NULL}, /* 82 */
{"bfalse",    3, NULL}, /* 83 */
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
{"malloc",    1, malloc_96}, /* 96 */
{"mmexa",     1, NULL}, /* 97 */
{"mmexa",     2, NULL}, /* 98 */
{"atraits",   1, NULL}, /* 99 */
{"dtraits",   1, NULL}, /* 100 */
{"rannots",   1, NULL}, /* 101 */
{"towner",    1, NULL}, /* 102 */
{"lock",      1, NULL}, /* 103 */
{"unlock",    1, NULL}, /* 104 */
{"wait",      1, NULL}, /* 105 */
{"sig",       1, NULL}, /* 106 */
{"sigall",    1, NULL}, /* 107 */
{"108",       0, NULL},        /* 108 */
{"109",       0, NULL},        /* 109 */
{"110",       0, NULL},        /* 110 */
{"111",       0, NULL},        /* 111 */
{"ldfunc",    3, ldfunc_112},  /* 112 */
{"ldfunc",    1, NULL}, /* 113 */
{"call",      2, call_114},    /* 114 */
{"calln",     2, NULL}, /* 115 */
{"ret",       1, ret_116},     /* 116 */
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
{"throw",     1, NULL}, /* 128 */
{"retrieve",  1, NULL}, /* 129 */
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
{"initProc",  2, NULL}, /* 144 */
{"join",      1, NULL}, /* 145 */
{"join2",     1, NULL}, /* 146 */
{"whoami",    1, NULL}, /* 147 */
};


/*
 * reverseEndianness
 *
 * This was a weird hack that I should have never used.
 * Originally I was reading in the ints backwards with 
 * the old vm520 read byte function then reversing their
 * endianness. Don't ask why, I don't know.
 */
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
#if DEBUG_XPVM
        fprintf( stderr, "Freeing stuff inside block.\n");
#endif
        //block *b = ((block**) CAST_INT regs[BLOCK_REG])[i];
        block *b = ((block**) CAST_INT blockPtr)[i];
        if( b )
        {
#if DEBUG_XPVM > 1
          fprintf( stderr, "b->data: %p\n", b->data );
          fprintf( stderr, "b->aux_data: %p\n", b->aux_data );
#endif
          free( b->data );
          free( b->aux_data );
          free( b );
        }
      }
      free( (block**) CAST_INT blockPtr );
    }
  }
}

/*********************************************************************
 * implementation of the public interface to the VM
 */

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
  //uint64_t objLength = 0;
  const uint32_t MAGIC = 0x31303636;
  uint32_t magic = 0;
  int i = 0;
  atexit( cleanup );

  CIO = 0;
  CIB = 0;

  /* Open the file */
  fp = fopen( filename, "r" );
  if( NULL == fp )
  {
    *errorNumber = -1;
    return 0;
  }

  /* read headers */
  if( !readWordXPVM( fp, (uint32_t*) &magic ) )
  {
    *errorNumber = -3;
    return 0;
  }
#if DEBUG_XPVM
  fprintf( stderr, "magic: %x\n", magic );
#endif
  if( MAGIC != magic )
  {
    *errorNumber = -3;
    return 0;
  }
  if( !readWordXPVM( fp, (uint32_t *) &blockCount ) )
  {
    *errorNumber = -3;
    return 0;
  }

  if(!(blockPtr2 = calloc(blockCount, sizeof(uint64_t))))
    EXIT_WITH_ERROR("Error: malloc failed in loadObjectFileXPVM");

  if(!(blockPtr = (uint64_t) CAST_INT calloc( blockCount, sizeof(block*) )))
    EXIT_WITH_ERROR("Error: malloc failed in loadObjectFileXPVM");

#if DEBUG_XPVM
  fprintf(stderr, "blockCount: %d\n", blockCount );
#endif

  /* 
   * Get the length of the object file 
   * and do a sanity check on the format 
   * FIXME: This doesn't work.
   */
  /*
  if( !getObjLengthXPVM( fp, blockCount, &objLength ) )
  {
    *errorNumber = -3;
    return 0;
  }*/

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
 * FIXME FIXME FIXME: This is broken!
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
 * FIXME FIXME FIXME: This is broken!
 */
static int getObjLengthXPVM( FILE *fp,uint32_t blockCount, uint64_t *objLength )
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
    if( !readWordXPVM( fp, &cur_length ) )
      return 0;
#if DEBUG_XPVM
    fprintf( stderr, "cur_length: %d\n", cur_length );    
#endif
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
  char name[256];
  uint32_t length               = 0;
  uint32_t frame_size           = 0;
  uint32_t num_except_handlers  = 0;
  uint32_t num_outsymbol_refs   = 0;
  uint32_t length_aux_data      = 0;
  uint64_t annots               = 0;
  uint64_t owner                = 0;
  uint8_t  *b_data              = 0;
  uint8_t  temp                 = 0;

  /*block_w *w = calloc( 1, sizeof(block_w) );
  if( !w )
    EXIT_WITH_ERROR("Error: malloc failed in readBlockXPVM");
  w->type = 1;
  ((block_w**) CAST_INT blockPtr)[blockNum] = w;*/

  block *b = calloc( 1, sizeof(block) );
  if( !b )
    EXIT_WITH_ERROR("Error: malloc failed in readBlockXPVM");
  ((block**) CAST_INT blockPtr)[blockNum] = b;
  /* Finally put the function block in the container */
  //w->u.b = b;
  int i = 0;
  /*uint64_t temp = 0;*/
  /* Read name string from block */
  while( (name[i] = fgetc( fp )) && ++i < MAX_NAME_LEN );
  /* Name too long */
  if( MAX_NAME_LEN == i )
    return 0;

#if DEBUG_XPVM
  fprintf( stderr, "name: %s\n", name );
#endif

  /* Read trait annotations */
  for( i = 0; i < 8; i++ )
    annots |=  ( (uint64_t)fgetc( fp ) << (64 - (i+1)*8) );
  //FIXME
  b->annots = annots;

#if DEBUG_XPVM
  fprintf( stderr, "annots: %016llx\n", annots );
#endif

  /* Read frame size */
  for( i = 0; i < 4; i++ )
    frame_size |=  ( (uint32_t)fgetc( fp ) << (32 - (i+1)*8) );
  //FIXME
  b->frame_size = frame_size;

#if DEBUG_XPVM
  fprintf( stderr, "frame_size: %08x\n", frame_size );
#endif

  /* Read contents length */
  for( i = 0; i < 4; i++ )
    length |= ( (uint32_t)fgetc( fp ) << (32 - (i+1)*8) );
  //FIXME
  b->length = length;

#if DEBUG_XPVM
  fprintf( stderr, "length: %08x\n", length );
#endif

  /* Allocate contents */
  b->data = calloc( b->length, sizeof(char) );
  b_data = calloc( length + BLOCK_HEADER_LENGTH, sizeof(uint8_t) );
  blockPtr2[blockNum] = (uint64_t) CAST_INT (b_data + BLOCK_HEADER_LENGTH);
  if( !b_data && length )
  {
    fprintf( stderr, "Error: malloc failed in readBlockXPVM\n");
    exit(-1);
  }

  /* Read contents */
  uint8_t *b_data_read = b_data + BLOCK_HEADER_LENGTH;
  for( i = 0; i < length; i++ )
  {
    b_data_read[i] = fgetc( fp );
    b->data[i] = b_data_read[i];
#if DEBUG_XPVM
    fprintf( stderr, "%02x\n", b_data_read[i] );
#endif
  }

  /* Read number of exception handlers */
  for( i = 0; i < 4; i++ )
    num_except_handlers |= ( (uint32_t)fgetc( fp ) << (32 - (i+1)*8) );
  b->num_except_handlers = num_except_handlers;

#if DEBUG_XPVM
  fprintf( stderr, "num_except_handlers: %08x\n", num_except_handlers );
#endif

  /* Read number of outsymbol references */
  for( i = 0; i < 4; i++ )
    num_outsymbol_refs |= ( (uint32_t)fgetc( fp ) << (32 - (i+1)*8) );
  b->num_outsymbol_refs = num_outsymbol_refs;

#if DEBUG_XPVM
  fprintf( stderr, "num_outsymbol_refs: %08x\n", b->num_outsymbol_refs );
#endif

  /* Read length of auxiliary data */
  for( i = 0; i < 4; i++ )
    length_aux_data |= ( (uint32_t)fgetc( fp ) << (32 - (i+1)*8) );
  b->length_aux_data = length_aux_data;

#if DEBUG_XPVM
  fprintf( stderr, "length_aux_data: %08x\n", b->length_aux_data );
#endif

  /* Allocate auxiliary data */
  
  /*
  aux_data = calloc( b->length_aux_data, sizeof(char) );
  if( !b->aux_data && b->length_aux_data )
  {
    fprintf( stderr, "Error: malloc failed in readBlockXPVM\n");
    exit(-1);
  }*/

  /* Read auxiliary data */
#if DEBUG_XPVM
    fprintf( stderr, "Skipping aux data.\n");
#endif
  for( i = 0; i < length_aux_data; i++ )
  {
    temp = fgetc( fp );
#if DEBUG_XPVM
    fprintf( stderr, "%02x\n", temp );
#endif
  }
  /*
  *(uint32_t*) b_data         = length;
  *(uint32_t*) (b_data + 4)   = frame_size;
  *(uint32_t*) (b_data + 8)   = num_except_handlers;
  *(uint32_t*) (b_data + 12)  = num_outsymbol_refs;
  *(uint32_t*) (b_data + 16)  = length_aux_data;
  *(uint64_t*) (b_data + 20)  = annots;
  *(uint64_t*) (b_data + 28)  = owner;*/
  BLOCK_OWNER( b_data_read )            = owner;
  BLOCK_ANNOTS( b_data_read )           = annots;
  BLOCK_AUX_LENGTH( b_data_read )       = length_aux_data;
  BLOCK_OUT_SYM_REFS( b_data_read )     = num_outsymbol_refs;
  BLOCK_EXCEPT_HANDLERS( b_data_read )  = num_except_handlers;
  BLOCK_FRAME_SIZE( b_data_read )       = frame_size;
  BLOCK_LENGTH( b_data_read )           = length;


  /* TODO Add this block to the VM memory. */

#if DEBUG_XPVM
  fprintf( stderr, "------- Block %d successfully "
                   "read from object file. -------\n", 
                   blockNum );
#endif

  return 1;  
}

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
  free( pt );

  return 0;
}

/*********************************************************************
 * functions to read an object file
 */

/*
 * readWordXPVM
 *
 * Reads a word (32 bits) from the object file and
 * returns it through the second parameter.
 * returns 0 on EOF.
 */
static int readWordXPVM(FILE *fp, uint32_t *outWord)
{
  int c1 = getc(fp);
  int c2 = getc(fp);
  int c3 = getc(fp);
  int c4 = getc(fp);

  if (c4 == EOF)
  {
    return 0;
  }
  *outWord = ((c1 << 24) | (c2 << 16) | (c3 << 8) | c4);
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

void test_block_macros( uint64_t ptr_as_int )
{
  uint8_t *b = (uint8_t *) CAST_INT ptr_as_int;

  fprintf( stderr, "Block Headers.\n"
                   "owner:            %llx\n"
                   "annots:           %llx\n"
                   "aux_length:       %x\n"
                   "out_sym_refs:     %x\n"
                   "except_handlers:  %x\n"
                   "frame_size:       %x\n"
                   "length:           %x\n",
                   BLOCK_OWNER( b ),
                   BLOCK_ANNOTS( b ),
                   BLOCK_AUX_LENGTH( b ),
                   BLOCK_OUT_SYM_REFS( b ),
                   BLOCK_EXCEPT_HANDLERS( b ),
                   BLOCK_FRAME_SIZE( b ),
                   BLOCK_LENGTH( b ) );
}

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
  uint64_t reg[256];
  reg[BLOCK_REG] = blockPtr;
  //reg[BLOCK_REG] = (uint64_t) CAST_INT blockPtr2;
  test_block_macros( blockPtr2[0] );
  stackFrame *stack = calloc( 1, sizeof(stackFrame) );
  if( !stack )
    EXIT_WITH_ERROR("Error: malloc failed in fetchExecuteXPVM");
  /*stack->data = calloc( 1, sizeof(stackFrame) );
  if( !stack->data )
    EXIT_WITH_ERROR("Error: malloc failed in fetchExecuteXPVM");*/

  /*
  block_w *wrapper = calloc( 1, sizeof(block_w) );
  if( !wrapper )
    EXIT_WITH_ERROR("Error: malloc failed in fetchExecuteXPVM");
  wrapper->type = STACK_FRAME_BLOCK;
  wrapper->u.sf = stack->data;*/

  /* FIXME?: Will this always be correct?
   * Need more test cases and find where it breaks.
   * I'm fairly confident it will.
   */

  block *b = ((block**) CAST_INT reg[BLOCK_REG])[0];
  if( 0 < b->frame_size )
  {
    stack->block = calloc( 1, sizeof(block) );
    /* FIXME: Throw OutOfMemory Exception */
    if( !stack->block )
      EXIT_WITH_ERROR("Error: malloc failed in fetchExecuteXPVM\n");
    stack->block->length = b->frame_size;
    stack->block->data = calloc( b->frame_size, sizeof(char) );
  }
  //uint8_t *b = (uint8_t *) CAST_INT ((uint64_t*) CAST_INT reg[BLOCK_REG])[0];
  //if( 0 < BLOCK_FRAME_SIZE( b ) )
  //{
  //}

  
  /* If work is null, set to pc to the main function in
   * the object file. Otherwise set it to work 
   */
  if( !work )
  {
    PCX = (uint64_t) CAST_INT b->data;
    //reg[STACK_FRAME_REG] = (uint64_t) CAST_INT b;
  }
  else
  {
    PCX = work;
  }
  reg[STACK_FRAME_REG] = (uint64_t) CAST_INT stack->block;

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
    reg[i] = (uint64_t) CAST_INT ar2;
  }

  /*stackNode *stack = args->stack;
  uint64_t *reg = args->regs;*/

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
    unsigned char *pc = (unsigned char*) CAST_INT PCX;
    uint8_t c1 = pc[0];
    uint8_t c2 = pc[1];
    uint8_t c3 = pc[2];
    uint8_t c4 = pc[3];
#if DEBUG_XPVM
    uint32_t word = assembleInst( (unsigned char*) CAST_INT PCX );
    fprintf( stderr, "\tword: %08x\n", word );
#endif

    // update PC
    PCX += 4;

    // execute
    //unsigned char opcode = word & 0xFF;
    //uint32_t opcode = (word & 0xFF000000) >> 24;
    uint32_t opcode = c1;
#if DEBUG_XPVM
    fprintf( stderr, "\topcode: %d\n", opcode );
#endif
    if (opcode > MAX_OPCODE_XPVM) // illegal instruction
    {
      return  (void *) VM520_ILLEGAL_INSTRUCTION;
    }
    int32_t ret = opcodesXPVM[opcode].formatFunc(1, reg, &stack,
                                                 c1, c2, c3, c4 );
#if DEBUG_XPVM
    fprintf( stderr, "\tret: %d\n", ret );
#endif
    if (ret <= 0)
    {
      /* Check if ret was called */
      if( 0x74 == opcode )
      {
        r->retVal = reg[stack->retReg];
        stackFrame *oldFrame = stack;
        stack = stack->prev;
        free( oldFrame->block );
        //free( oldNode->data );
        free( oldFrame );
      }
      r->status = ret;
      pthread_exit( (void*) CAST_INT r );
      return (void *) ret;
    }
    else if (ret != 1)
      EXIT_WITH_ERROR("Error: Unexpected return value from formatFunc"
                      " in fetchExecuteXPVM\n");
#if DEBUG_XPVM
    fprintf( stderr, "------- end fetch/execute cycle -------\n");
#endif
  }
  
  /* won't reach here */
  return 0;
}

/***************** main function ********************/

int main( int argc, char **argv )
{
  /* error for functions returning from vm520 */
  int errorNumber = 0;
  int64_t ptr = 0;
  retStruct *r = NULL;
  uint64_t retVal = 0;

  if( argc != 2 )
    EXIT_WITH_ERROR("Usage: xpvm one_object_file.obj\n");

  pthread_mutex_init( &malloc_xpvm_mu, NULL );

  pthread_mutex_lock( &malloc_xpvm_mu );
  malloc_xpvm_init( 10000 );
  pthread_mutex_unlock( &malloc_xpvm_mu );


  if (!loadObjectFileXPVM(argv[1], &errorNumber))
    EXIT_WITH_ERROR("loadObjectFileXPVM fails with error %d\n", errorNumber );

  doInitProc( &ptr, 0, 0, NULL );

  pthread_t *pt = (pthread_t*)(uint32_t)ptr;

  doProcJoin( (uint64_t)(uint32_t)pt, &retVal );

  r = (retStruct*) (uint32_t) retVal;

  fprintf( stderr, "r->retVal: %lld\n", (uint64_t)r->retVal );
  fprintf( stderr, "r->status: %d\n", (int)r->status );

  if( 0 == strcmp( argv[1], "ret_42_malloc.obj" ) )
  {
    block *ret_b = (block*) CAST_INT r->retVal;
    fprintf( stderr, "%s\n", ret_b->data );
  }

  free( r );

  //pthread_mutex_lock( &malloc_xpvm_mu );
  //malloc_xpvm_die = 1;
  //pthread_cond_wait( &malloc_xpvm_ans_cv, &malloc_xpvm_mu );
  //pthread_mutex_unlock( &malloc_xpvm_mu );

  return 0;
}
