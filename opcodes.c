/*
 * opcodes.c
 *
 * Code file for the implementation of the opcodes
 * for the XPVM.
 *
 * Author: Jeffrey Picard
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "xpvm.h"

int ldb_2( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int ldb_3( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int lds_4( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int lds_5( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int ldi_6( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t const8 )
{
  reg[ri] = (int32_t) reg[rj] + const8;
  return 1;
}

int ldi_7( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int ldl_8( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t rk )
{

  return 1;
}

int ldl_9( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t const8 )
{
  block_w *w = (block_w*) CAST_INT reg[rj];
  switch( w->type )
  {
    case DATA_BLOCK:
      reg[ri] = (uint64_t) *(int32_t *)(w->u.db->data + const8);
      break;
    case STACK_FRAME_BLOCK:
      reg[ri] = (uint64_t) *(int32_t *)(w->u.sf->block->data + const8);
      break;
    case FUNCTION_BLOCK:
      reg[ri] = (uint64_t) *(int32_t *)(w->u.b->data + const8);
      break;
    default:
      EXIT_WITH_ERROR("Error: In ldl_8 bad block type.\n");
      break;
  }

  return 1;
}

int ldf_10( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int ldf_11( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int ldd_12( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int ldd_13( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int ldimm_14( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t opcode, uint8_t ri, uint8_t c3, uint8_t c4 )
{
  int16_t const16  = TWO_8_TO_16( c3, c4 );

  reg[ri] = (int64_t)const16;

  return 1;
}

int ldimm2_15( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int stb_16( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int stb_17( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int sts_18( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int sts_19( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int sti_20( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t rk )
{
  return 1;
}

int sti_21( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t const8 )
{
  block_w *w = (block_w*) CAST_INT reg[rj];
  switch( w->type )
  {
    case DATA_BLOCK:
      *(int32_t *)(w->u.db->data + const8) = reg[ri];
      break;
    case STACK_FRAME_BLOCK:
      *(int32_t *)(w->u.sf->block->data + const8) = reg[ri];
      break;
    default:
      EXIT_WITH_ERROR("Error: In sti_20 bad block type.\n");
      break;
  }

  return 1;
}

int stl_22( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int stl_23( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int stf_24( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int stf_25( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int std_26( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int std_27( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int addl_32( unsigned int proc_id, uint64_t *reg, stackNode **stack,
              uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t rk )
{
  reg[ri] = (long)reg[rj] + (long)reg[rk];
  return 1;
}

int addl_33( unsigned int proc_id, uint64_t *reg, stackNode **stack,
              uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int subl_34( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t rk )
{
  reg[ri] = (long)reg[rj] - (long)reg[rk];
  return 1;
}

int subl_35( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int mull_36( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t rk )
{
  reg[ri] = (long)reg[rj] * (long)reg[rk];
  return 1;
}

int mull_37( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int divl_38( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int divl_39( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int reml_40( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int reml_41( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int negl_42( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int addd_43( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int subd_44( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int muld_45( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int divd_46( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int negd_47( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int cvtld_48( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int cvtdl_49( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int lshift_50( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int lshift_51( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int rshift_52( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int rshift_53( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int rshiftu_54( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int rshiftu_55( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int and_56( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int or_57( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int xor_58( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int ornot_59( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int cmpeq_64( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int cmpeq_65( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int cmple_66( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int cmple_67( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int cmplt_68( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int cmplt_69( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int cmpule_70( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int cmpule_71( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int cmpult_72( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int cmpult_73( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int fcmpeq_74( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int fcmple_75( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int fcmplt_76( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int jmp_80( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int jmp_81( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int btrue_82( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int bfalse_83( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int malloc_96( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int mmexa_97( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int mmexa_98( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int atraits_99( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int dtraits_100( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int rannots_101( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int towner_102( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int lock_103( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int unlock_104( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int wait_105( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int sig_106( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int sigall_107( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int ldfunc_112( unsigned int proc_id, uint64_t *reg, stackNode **stack,
                uint8_t opcode, uint8_t ri, uint8_t c3, uint8_t c4 )
{
  //uint16_t const16 = ((uint16_t)c3 << 8) | c4;
  uint16_t const16 = TWO_8_TO_16( c3, c4 );
  /*FIXME: Check index against f_block count */
  block_w *w = *(((block_w **) CAST_INT reg[BLOCK_REG]) + const16); 
#if DEBUG_XPVM
  fprintf( stderr, "\tldfunc: w: %p\n", w );
#endif
  reg[ri] = (uint64_t) CAST_INT w;
  return 1;
}

int ldfunc_113( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int call_114( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t const8 )
{
  block_w *func_wrap = (block_w*) CAST_INT reg[rj];
  f_block *b = func_wrap->u.b;
#if DEBUG_XPVM
  fprintf( stderr, "\tcall: b: %p\n", b );
#endif
  /*FIXME: Check annots for executable */
  stackFrame *f = calloc( 1, sizeof(stackFrame) );
  /* FIXME: Throw OutOfMemory Exception */
  if( !f )
    EXIT_WITH_ERROR("Error: malloc failed in call_114\n");
  f->pc = reg[PC_REG];
  f->cio = CIO;
  f->cib = CIB;
  f->reg255 = reg[255];
  f->retReg = ri;

  uint32_t frame_size = b->frame_size;
  if( frame_size )
  {
    f->block = calloc( 1, sizeof(d_block) + frame_size );
    /* FIXME: Throw OutOfMemory Exception */
    if( !f->block )
      EXIT_WITH_ERROR("Error: malloc failed in call_114\n");
    f->block->length = frame_size;

    /*
    f->locals = calloc( frame_size, sizeof(char) );
    if( !f->locals )
      EXIT_WITH_ERROR("Error: malloc failed in call_144\n");*/
  }
  stackNode *new_node = calloc( 1, sizeof(stackNode) );
  /* FIXME: Throw OutOfMemory Exception */
  if( !new_node )
    EXIT_WITH_ERROR("Error: malloc failed in call_114\n");
  new_node->data = f;
  new_node->prev = *stack;
  *stack = new_node;
  block_w *frame_wrap = (block_w *) CAST_INT reg[255];
  frame_wrap->u.sf = f;
  reg[PC_REG] = (uint64_t) CAST_INT b->data;

  return 1;
}

int calln_115( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int ret_116( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t opcode, uint8_t c2, uint8_t rj, uint8_t c4 )
{
#if DEBUG_XPVM > 1
  fprintf( stderr, "\treg: %p\n", reg );
  fprintf( stderr, "\tstack: %p\n", *stack );
#endif
  reg[(*stack)->data->retReg] = reg[rj];
  //pcXPVM = (unsigned char*) CAST_INT stack->data->pc;
  reg[PC_REG] = (*stack)->data->pc;
  reg[255] = (*stack)->data->reg255;
  CIO = (*stack)->data->cio;
  CIB = (*stack)->data->cib;
  /* pop frame */
#if DEBUG_XPVM
  fprintf( stderr, "\tretReg: %lld\n", (*stack)->data->retReg );
  fprintf( stderr, "\treg[retReg]: %lld\n", reg[(*stack)->data->retReg] );
#endif
  /* popped last stack, halt */
  if( !(*stack)->prev )
    return 0;
  stackNode *oldNode = *stack;
  *stack = (*stack)->prev;
  free( oldNode->data->block );
  free( oldNode->data );
  free( oldNode );

  return 1;
}

int throw_128( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int retrieve_129( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int initProc_144( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int join_145( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int join2_146( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int whoami_147( unsigned int proc_id, uint64_t *reg, stackNode **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}
