/*
 * xpvm.h
 *
 * Public interface to the XPVM implementation.
 *
 * Author: Jeffrey Picard
 */

#ifndef __XPVM_H
#define __XPVM_H


/* 32-bit and 64-bit types */
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <pthread.h>

/*************************** Macros ********************************/

#define DEBUG_XPVM  0
#define TRACK_EXEC  0

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

#define RET_OPCODE 0x74

#define CAST_INT (uint64_t)

#define u64 uint64_t

/* Exception Table */
#define SOFTWARE_DEFINED          0x0
#define ILLEGAL_INSTRUCTION       0x1
#define ILLEGAL_MEMORY_ADDRESS    0x2
#define ILLEGAL_MEMORY_REFERENCE  0x3
#define ILLEGAL_MEMORY_OPERATION  0x4
#define DIVIDE_BY_ZERO            0x5
#define OUT_OF_MEMORY             0x6
#define BAD_BLOCK_ID              0x7
#define ILLEGAL_CHAINING          0x8
#define ALREADY_OWNER             0x9
#define NOT_THE_OWNER             0xa

/* FIXME: This relies on the index variable i already being defined
 * This read a 32 bit integer from the object file INTO a little endian
 * 32 bit integer */
#define READ_INT32_LITTLE_ENDIAN( var, fp ) do {      \
  for( i = 0; i < 4; i++ )                            \
    var |= ( CAST_INT fgetc(fp) << (32 - (i+1)*8) );  \
} while(0)

#define READ_INT64_LITTLE_ENDIAN( var, fp ) do {      \
  for( i = 0; i < 8; i++ )                            \
    var |= ( CAST_INT fgetc(fp) << (64 - (i+1)*8) );  \
} while(0)

#define TWO_8_TO_16( b1, b2 ) ((uint16_t)b1 << 8) | b2

#define EXIT_WITH_ERROR(...)do {  \
  fprintf( stderr, __VA_ARGS__ ); \
  exit( -1 );                     \
}while(0)                         \

#define MALLOC_CHECK( p, ... ) do {  \
  if( !p )                           \
    EXIT_WITH_ERROR( __VA_ARGS__ );  \
} while(0)

#define DEBUG_PRINT(...)do {            \
  if( DEBUG_XPVM )                      \
    fprintf( stderr, __VA_ARGS__ );     \
} while(0)

/* Definition for block types */
//#define FUNCTION_BLOCK 1
//#define DATA_BLOCK 2
//#define STACK_FRAME_BLOCK 3

// maximum number of processors
#define XPVM_MAX_PROCESSORS 16

// error codes for load_object_file
#define XPVM_FILE_NOT_FOUND -1
#define XPVM_FILE_CONTAINS_OUTSYMBOLS -2
#define XPVM_FILE_IS_NOT_VALID -3

// error codes for execute
//   XPVM_ADDRESS_OUT_OF_RANGE and XPVM_ILLEGAL_INSTRUCTION are also
//     used by disassemble
#define XPVM_NORMAL_TERMINATION 0
#define XPVM_DIVIDE_BY_ZERO -1
#define XPVM_ADDRESS_OUT_OF_RANGE -2
#define XPVM_ILLEGAL_INSTRUCTION -3

/****************** Public Interface Functions **********************/

/* obj_file.c */
int32_t load_object_file( char *filename, int32_t *errorNumber, uint32_t*, uint64_t** );
int read_block( FILE *fp, int block_num, uint64_t* );
int read_word(FILE *fp, uint32_t *out_word);
int verify_obj_format(char *file_name, uint64_t *obj_len);

int do_init_proc( uint64_t *proc_id, uint64_t work, int argc, 
                 uint64_t *reg_bank );
int do_proc_join( uint64_t proc_id, uint64_t *ret_val );
int do_join2( uint64_t proc_id, uint64_t *ret_val );


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
  uint64_t      ret_reg;
  uint64_t      except_num;
  uint64_t      payload;
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

/******************** Other functions *******************************/

int process_exception( unsigned int proc_id, uint64_t *reg, stack_frame **stack, 
                       uint64_t except_num, uint64_t payload );

/*
 * Macros to access the array implementation of the blocks.
 * Each of these take a uint8_t (unsigned char), access it
 * at a negative index where the headers are stored and evaluate
 * to the requested header info.
 */
#define BLOCK_OWNER( b ) *(uint64_t*)(b - 8)
#define BLOCK_CHAIN( b ) *(uint64_t*)(b - 16)
#define BLOCK_ANNOTS( b ) *(uint64_t*)(b - 24)
#define BLOCK_AUX_LENGTH( b ) *(uint32_t*)(b - 24)
#define BLOCK_OUT_SYM_REFS( b ) *(uint32_t*)(b - 28)
/* Pointer to allocated chunk of memory, or null */
#define BLOCK_EXCEPT_HANDLERS( b ) *(uint64_t*)(b - 34)
/* Pointer to allocated chunk of memory, or null */
#define BLOCK_NATIVE_REFS( b ) *(uint64_t*)(b - 44)
#define BLOCK_FRAME_SIZE( b ) *(uint32_t*)(b - 48)
#define BLOCK_LENGTH( b ) *(uint32_t*)(b - 52)

#define BLOCK_HEADER_LENGTH 52

/*
 * Macros defining masks used in checking annotations
 */
#define MX_PRIVATE_MASK   0x0100000000000000
#define MX_RD_ONLY_MASK   0x0200000000000000
#define MX_LCK_RQD_MASK   0x0300000000000000

#define UNOWNABLE_MASK    0x0000000000000001
#define INST_MASK         0x0000000000000002
#define CHAINED_MASK      0x0000000000000004
#define OWNED_MASK        0x0000000000000008
#define PRIVATE_MASK      0x0000000000000010 
#define VOLATILE_MASK     0x0000000000000020 

#define CHECK_INST_ANNOT( b ) INST_MASK & BLOCK_ANNOTS( b )
#define CHECK_EXEC( b ) INST_MASK & BLOCK_ANNOTS( b )
#define CHECK_PRIVATE( b ) PRIVATE_MASK & BLOCK_ANNOTS( b )
#define CHECK_OWNED( b ) OWNED_MASK & BLOCK_ANNOTS( b )
#define CHECK_FREE( b ) !(OWNED_MASK & BLOCK_ANNOTS( b ))
#define CHECK_VOLATILE( b ) VOLATILE_MASK & BLOCK_ANNOTS( b )
#define CHECK_OWNED_VOLATILE(b) CHECK_OWNED(b) && CHECK_VOLATILE(b)
#define CHECK_FREE_VOLATILE(b) CHECK_FREE(b) && CHECK_VOLATILE(b)

#if CHECKS
/*
 * Macros for checking annotations
 */
#define CHECK_READ_ANNOTS( pid, b ) do {                        \
  if( CHECK_PRIVATE(b) && !(pid == BLOCK_OWNER(b)) ) {          \
    DEBUG_PRINT("Error: proc %d attempted to read "             \
                    "block %p illegally\n",                     \
                    pid, b );                                   \
    return process_exception( pid, reg, stack, ILLEGAL_MEMORY_OPERATION, 0);   \
  }                                                             \
  if( CHECK_OWNED(b) && !(pid == BLOCK_OWNER(b)) ) {            \
    DEBUG_PRINT("Error: proc %d attempted to read "             \
                    "block %p illegally\n",                     \
                    pid, b );                                   \
    return process_exception( pid, reg, stack, ILLEGAL_MEMORY_OPERATION, 0);   \
  }                                                             \
  if( CHECK_FREE(b) && !(CHECK_VOLATILE(b)) ) {                 \
    DEBUG_PRINT("Error: proc %d attempted to read "             \
                    "block %p illegally\n",                     \
                    pid, b );                                   \
    return process_exception( pid, reg, stack, ILLEGAL_MEMORY_OPERATION, 0);   \
  }                                                             \
}while(0)

#define CHECK_WRITE_ANNOTS( pid, b ) do {                       \
  if( CHECK_PRIVATE(b) && !(pid == BLOCK_OWNER(b)) ) {          \
    DEBUG_PRINT("Error: proc %d attempted to write "            \
                    "block %p illegally\n",                     \
                    pid, b );                                   \
    return process_exception( pid, reg, stack, ILLEGAL_MEMORY_OPERATION, 0);   \
  }                                                             \
  if( CHECK_OWNED(b) && !(pid == BLOCK_OWNER(b)) ) {            \
    DEBUG_PRINT("Error: proc %d attempted to write "            \
                    "block %p illegally\n",                     \
                    pid, b );                                   \
    return process_exception( pid, reg, stack, ILLEGAL_MEMORY_OPERATION, 0);   \
  }                                                             \
  if( CHECK_FREE(b) ) {                                         \
    DEBUG_PRINT("Error: proc %d attempted to write "            \
                    "block %p illegally\n",                     \
                    pid, b );                                   \
    return process_exception( pid, reg, stack, ILLEGAL_MEMORY_OPERATION, 0);   \
  }                                                             \
}while(0)

#define CHECK_EXEC_ANNOTS(pid, b) do {                          \
  if( !(CHECK_EXEC(b)) ) {                                      \
    DEBUG_PRINT("Error: proc %d attempted to execute "          \
                    "block %p illegally\n",                     \
                    pid, b );                                   \
    return process_exception( pid, reg, stack, ILLEGAL_MEMORY_OPERATION, 0);   \
  }                                                             \
}while(0);

#define CHECK_RELEASE_ANNOTS( pid, b ) do {                     \
  if( ! valid_bid((u64)b) ) {                                   \
    DEBUG_PRINT("Error: %p is not a valid block\n",             \
                    b);                                         \
    return process_exception( pid, reg, stack, BAD_BLOCK_ID, 0);   \
  }                                                             \
  if( CHECK_FREE(b) ) {                                         \
    DEBUG_PRINT("Error: proc %d attempted to release "          \
                    "free block %p\n",                          \
                    pid, b );                                   \
    return process_exception( pid, reg, stack, NOT_THE_OWNER, 0);   \
  }                                                             \
  if( CHECK_OWNED(b) && !(pid == BLOCK_OWNER(b)) ) {            \
    DEBUG_PRINT("Error: proc %d attempted to realse "           \
                    "block %p illegally\n",                     \
                    pid, b );                                   \
    return process_exception( pid, reg, stack, NOT_THE_OWNER, 0);   \
  }                                                             \
}while(0)

#define CHECK_AQUIRE_ANNOTS( pid, b ) do {                      \
  if( ! valid_bid((u64)b) ) {                                   \
    DEBUG_PRINT("Error: %p is not a valid block\n",             \
                    b);                                         \
    return process_exception( pid, reg, stack, BAD_BLOCK_ID, 0);   \
  }                                                             \
  if( CHECK_OWNED(b) && (pid == BLOCK_OWNER(b)) ) {             \
    DEBUG_PRINT("Error: proc %d already owns block %p\n",       \
                    pid, b );                                   \
    return process_exception( pid, reg, stack, ALREADY_OWNER, 0);   \
  }                                                             \
  if( CHECK_PRIVATE (b) ) {                                     \
    DEBUG_PRINT("Error: proc %d attempted to access "           \
                    "private block %p\n",                       \
                    pid, b );                                   \
    return process_exception( pid, reg, stack, ILLEGAL_MEMORY_OPERATION, 0);   \
  }                                                             \
} while(0)

#else //CHECKS

/*#define CHECK_INST_ANNOT( b ) {}
#define CHECK_PRIVATE( b ) {}
#define CHECK_OWNED( b ) {}
#define CHECK_FREE( b ) {}
#define CHECK_VOLATILE( b ) {}
#define CHECK_OWNED_VOLATILE(b) {}
#define CHECK_FREE_VOLATILE(b) {}*/

#define CHECK_READ_ANNOTS( pid, b ) {}
#define CHECK_WRITE_ANNOTS( pid, b ) {}
#define CHECK_EXEC_ANNOTS(pid, b) {}
#define CHECK_RELEASE_ANNOTS(pid, b) {}
#define CHECK_AQUIRE_ANNOTS(pid, b) {}

#endif //CHECKS


/*
 * Macros for setting annotations
 */
#define SET_BLOCK_OWNED( b ) do {                     \
  BLOCK_ANNOTS( b ) = BLOCK_ANNOTS( b ) | OWNED_MASK; \
} while(0)

#define SET_BLOCK_FREE( b ) do {                        \
  BLOCK_ANNOTS( b ) = BLOCK_ANNOTS( b ) & ~OWNED_MASK;  \
} while(0)

#define SET_BLOCK_VOLATILE( b ) do {                     \
  BLOCK_ANNOTS( b ) = BLOCK_ANNOTS( b ) | VOLATILE_MASK; \
} while(0)

#define SET_BLOCK_PRIVATE( b ) do {                   \
  BLOCK_ANNOTS(b) = BLOCK_ANNOTS(b) | PRIVATE_MASK;   \
} while(0)

#define SET_BLOCK_CHAINED( b, id ) do {             \
  BLOCK_ANNOTS(b) = BLOCK_ANNOTS(b) | CHAINED_MASK; \
  if( ! valid_bid( id ) )                           \
    DEBUG_PRINT("Error: Invalid block ID!\b");  \
  uint64_t p_id = id;                               \
  while( BLOCK_CHAIN(p_id) )                        \
    p_id = BLOCK_CHAIN(p_id);                       \
  BLOCK_CHAIN(b) = p_id;                            \
} while(0)

/*
 * CMPXCHG
 * Compare exchange macro for aquire block.
 */
int aquire_blk_asm( uint64_t*, uint32_t );
#define CMPXCHG( own_ptr, nil, new_owner ) do {     \
  if( aquire_blk_asm( own_ptr, new_owner ) )        \
    reg[rk] = 1;                                    \
  else                                              \
    reg[rk] = 0;                                    \
}while(0)

/*
 * Struct to hold the arguments passed to doProcInit.
 */
struct _cmd_arg
{
  char s[0];
} typedef cmd_arg;

/*
 * Struct for passing back the exit status and return value
 * of a processor (thread)i from doJoin.
 */
struct _ret_struct
{
  int status;
  int64_t ret_val;
} typedef ret_struct;

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

int ldb_2                     OPCODE_FUNC
int ldb_3                     OPCODE_FUNC
int lds_4                     OPCODE_FUNC
int lds_5                     OPCODE_FUNC
int ldi_6                     OPCODE_FUNC
int ldi_7                     OPCODE_FUNC
int ldl_8                     OPCODE_FUNC
int ldl_9                     OPCODE_FUNC
int ldimm_14                  OPCODE_FUNC
int ldimm2_15                 OPCODE_FUNC
int stb_16                    OPCODE_FUNC
int stb_17                    OPCODE_FUNC
int sti_21                    OPCODE_FUNC
int ldblkid_28                OPCODE_FUNC
int addl_32                   OPCODE_FUNC
int addl_33                   OPCODE_FUNC
int subl_34                   OPCODE_FUNC
int subl_35                   OPCODE_FUNC
int mull_36                   OPCODE_FUNC
int mull_37                   OPCODE_FUNC
int divl_38                   OPCODE_FUNC
int divl_39                   OPCODE_FUNC
int reml_40                   OPCODE_FUNC
int reml_41                   OPCODE_FUNC
int negl_42                   OPCODE_FUNC
int addd_43                   OPCODE_FUNC
int subd_44                   OPCODE_FUNC
int muld_45                   OPCODE_FUNC
int divd_46                   OPCODE_FUNC
int negd_47                   OPCODE_FUNC
int cvtld_48                  OPCODE_FUNC
int cvtdl_49                  OPCODE_FUNC
int lshift_50                 OPCODE_FUNC
int lshift_51                 OPCODE_FUNC
int rshift_52                 OPCODE_FUNC
int rshift_53                 OPCODE_FUNC
int rshiftu_54                OPCODE_FUNC
int rshiftu_55                OPCODE_FUNC
int and_56                    OPCODE_FUNC
int or_57                     OPCODE_FUNC
int xor_58                    OPCODE_FUNC
int ornot_59                  OPCODE_FUNC
int cmplt_68                  OPCODE_FUNC
int jmp_80                    OPCODE_FUNC
int btrue_82                  OPCODE_FUNC
int bfalse_83                 OPCODE_FUNC
int alloc_blk_96              OPCODE_FUNC
int alloc_private_blk_97      OPCODE_FUNC
int aquire_blk_98             OPCODE_FUNC
int release_blk_99            OPCODE_FUNC
int ldfunc_112                OPCODE_FUNC
int call_114                  OPCODE_FUNC
int calln_115                 OPCODE_FUNC
int ret_116                   OPCODE_FUNC
int init_proc_144             OPCODE_FUNC
int join_145                  OPCODE_FUNC

/*************************** Native functions ****************************/

#endif
