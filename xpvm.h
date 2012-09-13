/*
 * xpvm.h
 *
 * Public interface to the XPVM implementation.
 *
 * Author: Jeffrey Picard
 */

#ifndef __VM520_H
#define __VM520_H

#include <stdint.h>
#include <pthread.h>

/*************************** Macros ********************************/

#define DEBUG_XPVM 1

#define MAX_REGS      256
#define HIDDEN_REGS   4
#define NUM_REGS      MAX_REGS + HIDDEN_REGS
#define MAX_NAME_LEN  256

//uint64_t CIO;
//uint64_t CIB;

#define PCX reg[253]
#define PC_REG 253
#define CIO_REG 256
#define CIO reg[CIO_REG]
#define CIB_REG 257
#define CIB reg[CIB_REG]
#define BLOCK_REG 258
#define STACK_FRAME_REG 255

#define CAST_INT (uint32_t)

#define TWO_8_TO_16( b1, b2 ) ((uint16_t)b1 << 8) | b2

#define EXIT_WITH_ERROR(...){     \
  fprintf( stderr, __VA_ARGS__ ); \
  exit( -1 );                     \
}                                 \

/* Definition for block types */
//#define FUNCTION_BLOCK 1
//#define DATA_BLOCK 2
//#define STACK_FRAME_BLOCK 3

// maximum number of processors
#define VM520_MAX_PROCESSORS 16

// error codes for loadObjectFile
#define VM520_FILE_NOT_FOUND -1
#define VM520_FILE_CONTAINS_OUTSYMBOLS -2
#define VM520_FILE_IS_NOT_VALID -3

// error codes for execute
//   VM520_ADDRESS_OUT_OF_RANGE and VM520_ILLEGAL_INSTRUCTION are also
//     used by disassemble
#define VM520_NORMAL_TERMINATION 0
#define VM520_DIVIDE_BY_ZERO -1
#define VM520_ADDRESS_OUT_OF_RANGE -2
#define VM520_ILLEGAL_INSTRUCTION -3

/****************** Public Interface Functions **********************/

int32_t loadObjectFileXPVM( char *filename, int32_t *errorNumber );

int doInitProc( int64_t *retVal, uint64_t work, int argc, 
                 uint64_t *reg_bank );
int doProcJoin( uint64_t procID, uint64_t *retVal );

/************************** Structs *********************************/

/*
 * Struct for a stack frame in the VM.
 */
typedef struct _block block;
struct _stack_frame
{
  uint64_t      pc;
  uint64_t      cio;
  uint64_t      cib;
  uint64_t      reg255;
  uint64_t      retReg;
  //unsigned char *locals;
  //block         *block;
  struct _stack_frame *prev;
  uint8_t     *block;
} typedef stack_frame;

/*
 * Struct for the VM stack.
 */
/*
struct _stackNode
{
  stack_frame        *data;
  struct _stackNode *prev;
} typedef stackNode;*/

/*
 * Struct for the blocks read into memory from the object file.
 */
struct _block
{
  uint32_t      length;
  uint32_t      frame_size;
  uint32_t      num_except_handlers;
  uint32_t      num_outsymbol_refs;
  uint32_t      length_aux_data;
  uint64_t      annots;
  uint64_t      owner;
  //char          name[256];
  uint8_t *data;
  uint8_t *aux_data;
  //struct _block *next;
} typedef block;

/*
 * Macros to access the array implementation of the blocks.
 * Each of these take a uint8_t (unsigned char), access it
 * at a negative index where the headers are stored and evaluate
 * to the requested header info.
 */
#define BLOCK_OWNER( b ) *(uint64_t*)(b - 8)
#define BLOCK_ANNOTS( b ) *(uint64_t*)(b - 16)
#define BLOCK_AUX_LENGTH( b ) *(uint32_t*)(b - 20)
#define BLOCK_OUT_SYM_REFS( b ) *(uint32_t*)(b - 24)
#define BLOCK_EXCEPT_HANDLERS( b ) *(uint32_t*)(b - 28)
#define BLOCK_FRAME_SIZE( b ) *(uint32_t*)(b - 32)
#define BLOCK_LENGTH( b ) *(uint32_t*)(b - 36)

#define BLOCK_HEADER_LENGTH 36

/*
 * Macros defining masks used in checking annotations
 */
#define MX_PRIVATE_MASK   0x0100000000000000
#define MX_RD_ONLY_MASK   0x0200000000000000
#define MX_LCK_RQD_MASK   0x0300000000000000

#define UNKNOWABLE_MASK   0x0000000000000001
#define INST_MASK         0x0000000000000002

/*
 * Macros for checking annotations
 */
#define CHECK_INST_ANNOT( b ) INST_MASK & BLOCK_ANNOTS( b )

/*
 * Struct to hold the arguments passed to doProcInit.
 */
struct _cmdArg
{
  char s[0];
} typedef cmdArg;

/*
 * Struct for passing back the exit status and return value
 * of a processor (thread)i from doJoin.
 */
struct _retStruct
{
  int status;
  int64_t retVal;
} typedef retStruct;

/*
 * Struct for the arguments to the fetch_execute function
 * which is the work function passed to the pthread.
 */
struct _fe_args
{
  uint64_t *reg_bank;
  uint64_t work;
  int argc;
} typedef fe_args;

/*
 * Allocator function and mutex.
 */
pthread_mutex_t malloc_xpvm_mu;
int malloc_xpvm_init( uint64_t );
uint64_t malloc_xpvm( uint32_t );

/*
 * Dynamic link handle.
 */
#define C_LIB_PATH "./wrapped_c_lib.so"
void *__lh;

/********************** Opcode Declarations **************************/

#define OPCODE_FUNC ( unsigned int proc_id, uint64_t *reg, stack_frame **stak, \
                    uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 );

int ldb_2       OPCODE_FUNC
int ldb_3       OPCODE_FUNC
int lds_4       OPCODE_FUNC
int lds_5       OPCODE_FUNC
int ldi_6       OPCODE_FUNC
int ldi_7       OPCODE_FUNC
int ldl_8       OPCODE_FUNC
int ldl_9       OPCODE_FUNC
int ldimm_14    OPCODE_FUNC
int ldimm2_15   OPCODE_FUNC
int stb_16      OPCODE_FUNC
int stb_17      OPCODE_FUNC
int sti_21      OPCODE_FUNC
int addl_32     OPCODE_FUNC
int subl_34     OPCODE_FUNC
int mull_36     OPCODE_FUNC
int malloc_96   OPCODE_FUNC
int ldfunc_112  OPCODE_FUNC
int call_114    OPCODE_FUNC
int calln_115   OPCODE_FUNC
int ret_116     OPCODE_FUNC

/*************************** Native functions ****************************/

#endif
