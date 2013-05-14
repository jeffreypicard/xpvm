/*
 * xpvm.c
 *
 * Implementation of the XPVM.
 *
 * Author: Jeffrey Picard
 * Based on Professor Hatcher's vm520.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <dlfcn.h>
#include <assert.h>

#include "xpvm.h"
#include "opcode_table.h"

/* Number of bytes for the XPVM memory allocator */
#define XPVM_MEM_SIZE 10000

/* forward references */
static void *fetch_execute(void *v);
void test_block_macros( uint64_t ptr_as_int );

/* linked list to keep track of (symbol,address) pairs for insymbols
static struct insymbol {
  char *symbol;
  unsigned int addr;
  struct insymbol *next;
} *insymbols = NULL;*/

uint64_t regs[NUM_REGS];
uint32_t block_cnt = 0;
uint64_t *block_ptr = 0;

/*
 * cleanup
 *
 * This function is passed to atexit in the load_object_file
 * function. It cleans up all the memory used when loading the
 * object file.
 * FIXME: This currently does not do a very good job at cleaning up
 * all the memory. Is it really even needed?
 */
void cleanup( void )
{
  int i = 0;
  if( block_cnt )
  {
    if( block_ptr )
    {
      for( i = 0; i < block_cnt; i++ )
      {
#if DEBUG_XPVM
        fprintf( stderr, "Freeing stuff inside block.\n");
#endif
        uint8_t *b = (uint8_t*) CAST_INT block_ptr[i];
        free( b - BLOCK_HEADER_LENGTH );
      }
      free( block_ptr );
    }
  }
  dlclose( __lh );
}

/*
 * load_c_lib
 * 
 * This function loads the avilable C library functions
 * from a dynamic library when the XPVM starts up.
 */
int load_c_lib( void )
{
  __lh = dlopen( C_LIB_PATH, RTLD_NOW ); 
  if( !__lh )
    EXIT_WITH_ERROR("Error: in load_c_libs, %s\n", dlerror() );

  /* Clear any previous errors */
  dlerror();

  return 0;
}

/*
 * process_exception
 *
 */
int process_exception( unsigned int proc_id, uint64_t *reg, stack_frame **stack, 
                       uint64_t except_num, uint64_t payload )
{
  int i;
  uint32_t *handlers    = 0;
  uint32_t num_handlers = 0;
  uint32_t start_offset = 0;
  uint32_t end_offset   = 0;
  uint32_t code_offset  = 0;
  while(1)
  {
    handlers = (uint32_t*) BLOCK_EXCEPT_HANDLERS( CIB );
    if( handlers )
    {
      /* Search for exception handler */
      num_handlers = *handlers;
      handlers++;

      for( i = 0; i < num_handlers; i++ )
      {
        start_offset  = *handlers++;
        end_offset    = *handlers++;
        code_offset   = *handlers++;
        if( start_offset <= CIO && end_offset >= CIO )
        {
          /* CIB is current block, CIO is the offset specified by the handler */
          CIO = code_offset;
          (*stack)->except_num = except_num;
          (*stack)->payload = payload;
          return 1;
        }
      }
    }
    /* Pop current stack and look for exception handlers in the next. */
    reg[PC_REG] = (*stack)->pc;
    reg[255] = (*stack)->reg255;
    CIO = (*stack)->cio;
    CIB = (*stack)->cib;
    /* popped last stack, halt and issue error. */
    if( !(*stack)->prev )
      return 2;
      //EXIT_WITH_ERROR("Error: Unhandled Exception\n");
    stack_frame *old_frame = *stack;
    *stack = (*stack)->prev;
    if( old_frame->block )
      free( old_frame->block - BLOCK_HEADER_LENGTH );
    free( old_frame );
    /* Set the CIO to the call that resulted in the exception */
    CIO = CIO - 4;
  }

  /* Should never be reached */
  assert(0);
  return 0;
}

/*********************************************************************
 * implementation of the public interface to the VM
 */

/*
 * do_init_proc
 *
 * C function implementation of the init_proc
 * opcode. also used to start the VM executing
 * the main function.
 */
int do_init_proc( uint64_t *proc_id, uint64_t work, int argc, 
                 uint64_t *reg_bank )
{
  pthread_t *pt = calloc( 1, sizeof(pthread_t) );

#if DEBUG_XPVM
  fprintf( stderr, "Starting processors.\n");
#endif

  fe_args *ar3 = calloc( 1, sizeof(fe_args) );
  if( !ar3 )
    EXIT_WITH_ERROR("Error: malloc failed in do_init_proc\n");
  ar3->reg_bank = reg_bank;
  ar3->work = work;
  ar3->argc = argc;

  if (pthread_create(pt, NULL, fetch_execute, (void *) ar3) != 0)
  {
    perror("error in thread create");
    exit(-1);
  }

  *proc_id = (uint64_t) CAST_INT pt;

  return 0;
}

int do_proc_join( uint64_t proc_id, uint64_t *ret_val )
{
  pthread_t *pt = (pthread_t*) CAST_INT proc_id;
  void *ret;
  if (pthread_join(*pt, &ret) != 0)
  {
    perror("error in thread join");
    exit(-1);
  }
  /**ret_val = (uint64_t) CAST_INT ret;*/
  *ret_val =  ((ret_struct*)ret)->ret_val;
  free( pt );

  return 0;
}

int do_join2( uint64_t proc_id, uint64_t *ret_val )
{
  pthread_t *pt = (pthread_t*) CAST_INT proc_id;
  void *ret;
  if (pthread_tryjoin_np(*pt, &ret) != 0)
  {
    perror("error in thread join");
    exit(-1);
  }
  *ret_val = (uint64_t) CAST_INT ret;
  //free( pt );

  return 0;
}

/*********************************************************************
 * functions to read an object file
 */

uint32_t assemble_inst( uint8_t *pc )
{
  uint8_t c1 = pc[0];
  uint8_t c2 = pc[1];
  uint8_t c3 = pc[2];
  uint8_t c4 = pc[3];
#if DEBUG_XPVM
  fprintf( stderr, "\tc1: %02x\n"
                   "\tc2: %02x\n"
                   "\tc3: %02x\n"
                   "\tc4: %02x\n",
                   c1,c2,c3,c4);
#endif
  uint32_t inst = (c1 << 24) | (c2 << 16) | (c3 << 8) | c4;
  return inst;
}

void test_block_macros( uint64_t ptr_as_int )
{
#if DEBUG_XPVM
  uint8_t *b = (uint8_t *) CAST_INT ptr_as_int;
  fprintf( stderr, "Block Headers.\n"
                   "owner:            %" PRIx64 "\n"
                   "annots:           %" PRIx64 "\n"
                   "aux_length:       %x\n"
                   "out_sym_refs:     %x\n"
                   "num_native_refs:  %" PRIx64 "\n"
                   "except_handlers:  %" PRIx64 "\n"
                   "frame_size:       %x\n"
                   "length:           %" PRIx32 "\n",
                   BLOCK_OWNER( b ),
                   BLOCK_ANNOTS( b ),
                   BLOCK_AUX_LENGTH( b ),
                   BLOCK_OUT_SYM_REFS( b ),
                   BLOCK_NATIVE_REFS( b ),
                   BLOCK_EXCEPT_HANDLERS( b ),
                   BLOCK_FRAME_SIZE( b ),
                   BLOCK_LENGTH( b ) );
#endif
}

/*
 * fetch_execute
 *
 * Get the next instruction and execute it.
 *
 */
static void *fetch_execute(void *v)
{
#if DEBUG_XPVM
  fprintf( stderr, "In fetch_execute.\n");
#endif
  /*int i = 0;*/
  fe_args *args = (fe_args*)v;
  uint64_t *reg_bank = args->reg_bank;
  int argc = args->argc;
  uint64_t work = args->work;

  free( args );

  int i = 0;
  int len = 0;
  cmd_arg *ar1 = NULL, *ar2 = NULL;
  /* Inialize the VM to run */
  uint64_t reg[NUM_REGS];
  reg[0] = 0;
  reg[1] = 0;
  reg[BLOCK_REG] = (uint64_t) CAST_INT block_ptr;
  //test_block_macros( block_ptr[0] );
  stack_frame *stack = calloc( 1, sizeof(stack_frame) );
  if( !stack )
    EXIT_WITH_ERROR("Error: malloc failed in fetch_execute");

  uint8_t *b = NULL;
  if( !work )
    b = (uint8_t *) CAST_INT ((uint64_t*) CAST_INT reg[BLOCK_REG])[0];
  else
    b = (uint8_t*) CAST_INT work;

  if( 0 < BLOCK_FRAME_SIZE( b ) )
  {
    stack->block = calloc( BLOCK_FRAME_SIZE( b ) + BLOCK_HEADER_LENGTH, 
                           sizeof(uint8_t) );
    // FIXME: Throw OutOfMemory Exception
    if( !stack->block )
      EXIT_WITH_ERROR("Error: malloc failed in fetch_execute\n");
    stack->block = stack->block + BLOCK_HEADER_LENGTH;
    BLOCK_LENGTH( stack->block ) = BLOCK_FRAME_SIZE( b );
  }

  
  PCX = (uint64_t) CAST_INT b;
  CIB = (uint64_t) CAST_INT b;
  CIO = 0;
  reg[STACK_FRAME_REG] = (uint64_t) CAST_INT stack->block;

  /* FIXME */
  /* This currently only supports 10 args */
  /* If work is NULL, this was started from the command line */
  if( !work )
  {
#if DEBUG_XPVM
    fprintf( stderr, "Started from command line\n" );
#endif
    for( i = 1; i <= argc && i < 11; i++ )
    {
      ar1 = ((cmd_arg*) CAST_INT (reg_bank+i));
      len = strlen( ar1->s );
      ar2 = calloc( 1, sizeof(cmd_arg) + len + 1 );
      if( !ar2 )
        EXIT_WITH_ERROR("Error: malloc failed in do_init_proc\n");
      strcpy( ar2->s, ar1->s );
      reg[i] = (uint64_t) CAST_INT ar2;
    }
  }
  else
  {
#if DEBUG_XPVM
    fprintf( stderr, "copying over %d args\n", argc );
#endif
    for( i = 1; i <= argc && i < 11; i++ )
    {
#if DEBUG_XPVM
      fprintf( stderr, "arg %d: %" PRIu64 "\n", i, reg_bank[i] );
#endif
      reg[i] = reg_bank[i];
    }
    free( reg_bank );
  }

  /* the processor ID is passed in */
  /*int processorID = args->procNum;*/
  ret_struct *r = calloc( 1, sizeof(ret_struct) );
  if( !r )
    EXIT_WITH_ERROR("Error: malloc failed in do_init_proc\n");

  /* fetch/execute cycle */
  while (1)
  {
#if TRACK_EXEC
    fprintf( stderr, "------- start fetch/execute cycle -------\n");
#endif
    /* check to see if the PC is in range
    if ((PC < 0) || (PC >= MAX_ADDRESS))
    {
      return (void *) XPVM_ADDRESS_OUT_OF_RANGE;
    }*/

    // fetch
    //int word = memory[PC];
    //uint8_t *pc = (unsigned char*) CAST_INT PCX;
    uint8_t *pc = ((uint8_t *) CAST_INT CIB) + CIO;
    uint8_t c1 = pc[0];
    uint8_t c2 = pc[1];
    uint8_t c3 = pc[2];
    uint8_t c4 = pc[3];
#if TRACK_EXEC
    uint32_t word = assemble_inst( ((unsigned char*) CAST_INT CIB) + CIO );
    fprintf( stderr, "\tword: %08x\n", word );
#endif

    // update PC
    //PCX += 4;
    if( CIO + 4 > BLOCK_LENGTH( (uint8_t *) CAST_INT CIB ) )
      EXIT_WITH_ERROR("Error: Instructions over ran CIB in fetch_execute!\n");
    CIO += 4;

    // execute
    //unsigned char opcode = word & 0xFF;
    //uint32_t opcode = (word & 0xFF000000) >> 24;
    uint32_t opcode = c1;
#if TRACK_EXEC
    fprintf( stderr, "\topcode: %d\n", opcode );
#endif
    if (opcode > MAX_OPCODE_XPVM) // illegal instruction
    {
      return  (void *) XPVM_ILLEGAL_INSTRUCTION;
    }

    int32_t ret = opcodes[opcode].formatFunc(1, reg, &stack,
                                                 c1, c2, c3, c4 );
#if TRACK_EXEC
    fprintf( stderr, "\tret: %d\n", ret );
#endif
    if (ret <= 0)
    {
      /* Check if ret was called */
      if( RET_OPCODE == opcode )
      {
        r->ret_val = reg[stack->ret_reg];
        stack_frame *old_frame = stack;
        stack = stack->prev;
        if( old_frame->block )
          free( old_frame->block - BLOCK_HEADER_LENGTH );
        //free( oldNode->data );
        free( old_frame );
      }
      r->status = ret;
      pthread_exit( (void*) CAST_INT r );
      return (void *) CAST_INT ret;
    }
    else if (ret == 2)
    {
      /*Uncaught exception*/
      EXIT_WITH_ERROR("Error: uncaught exception\n");
    }
    else if (ret != 1)
      EXIT_WITH_ERROR("Error: Unexpected return value from formatFunc"
                      " in fetch_execute\n");
#if TRACK_EXEC
    fprintf( stderr, "------- end fetch/execute cycle -------\n");
#endif
  }
  
  /* won't reach here */
  return 0;
}

/***************** main function ********************/

int main( int argc, char **argv )
{
  /* error for functions returning from XPVM */
  int error_num = 0;
  uint64_t ptr = 0;
  uint64_t obj_len = 0;
  ret_struct *r = NULL;
  /*uint64_t ret_val = 0;*/
  void *ret = NULL;

  if( argc != 2 )
    EXIT_WITH_ERROR("Usage: xpvm one_object_file.obj\n");

  regs[0] = 0;
  regs[1] = 0;

  pthread_mutex_init( &malloc_xpvm_mu, NULL );

  /* Initialize the allocator and the dynamic libraries */
  pthread_mutex_lock( &malloc_xpvm_mu );
  malloc_xpvm_init( XPVM_MEM_SIZE );
  load_c_lib();
  pthread_mutex_unlock( &malloc_xpvm_mu );

  if( verify_obj_format( argv[1], &obj_len ) != 0 )
    EXIT_WITH_ERROR("Error: Invalid of corrupt object file.\n");

  if (!load_object_file(argv[1], &error_num, &block_cnt, &block_ptr))
    EXIT_WITH_ERROR("Error: load_object_file failed with error %d\n", error_num );

  do_init_proc( &ptr, 0, 0, NULL );

  pthread_t *pt = (pthread_t*) CAST_INT ptr;

  /*do_proc_join( (uint64_t)(uint32_t)pt, &ret_val );*/
  /* Since this is the main process we need to return the whole struct
   * not just the 64 bit thing that it returned as its return value */
  if (pthread_join(*pt, &ret) != 0)
  {
    perror("error in thread join");
    exit(-1);
  }

  r =  (ret_struct*)ret;
  /*r = (ret_struct*) (uint32_t) ret;*/

  /* For floats. */
  fprintf( stderr, "r->ret_val: %1.8lf\n", *(double*)&(r->ret_val) );
  /*fprintf( stderr, "r->ret_val: %lld\n", (uint64_t)r->ret_val );*/
  /*fprintf( stderr, "r->status: %d\n", (int)r->status );*/

  /* FIXME: Double free when running with multiple processors */
  /*free( r );*/

  return 0;
}
