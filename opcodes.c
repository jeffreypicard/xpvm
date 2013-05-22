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
#include <dlfcn.h>

#include "xpvm.h"

char *native_funcs[] =
{
  "print_int",
  "print_string",
  "xpvm_printf",
};


int ldb_2( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t rk )
{
  uint8_t *b = (uint8_t*) CAST_INT reg[rj];
  CHECK_READ_ANNOTS( proc_id, b );
  reg[ri] = *(int8_t *)(b + reg[rk]);
  return 1;
}

int ldb_3( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t const8 )
{
  uint8_t *b = (uint8_t*) CAST_INT reg[rj];
  CHECK_READ_ANNOTS( proc_id, b );
  reg[ri] = *(int8_t *)(b + const8);
  return 1;
}

int lds_4( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t rk )
{
  uint8_t *b = (uint8_t*) CAST_INT reg[rj];
  CHECK_READ_ANNOTS( proc_id, b );
  reg[ri] = *(int16_t *)(b + reg[rk]);
  return 1;
}

int lds_5( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t const8 )
{
  uint8_t *b = (uint8_t*) CAST_INT reg[rj];
  CHECK_READ_ANNOTS( proc_id, b );
  reg[ri] = *(int16_t *)(b + const8);
  return 1;
}

int ldi_6( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t const8 )
{
  uint8_t *b = (uint8_t*) CAST_INT reg[rj];
  CHECK_READ_ANNOTS( proc_id, b );
  reg[ri] = *(int32_t *)(b + const8);
  return 1;
}

int ldi_7( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t rk )
{
  uint8_t *b = (uint8_t*) CAST_INT reg[rj];
  CHECK_READ_ANNOTS( proc_id, b );
  reg[ri] = (int64_t) *(int32_t *)(b + reg[rk]);
  return 1;
}

int ldl_8( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t rk )
{
  uint8_t *b = (uint8_t*) CAST_INT reg[rj];
  CHECK_READ_ANNOTS( proc_id, b );
  reg[ri] = *(int64_t *)(b + reg[rk]);

  return 1;
}

int ldl_9( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t const8 )
{
  uint8_t *b = (uint8_t*) CAST_INT reg[rj];
  CHECK_READ_ANNOTS( proc_id, b );
  reg[ri] = *(int64_t*)(b + const8);

  return 1;
}

int ldf_10( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t rk )
{
  uint8_t *b = (uint8_t*) CAST_INT reg[rj];
  CHECK_READ_ANNOTS( proc_id, b );
  reg[ri] = *(uint64_t*)(float*)(b + reg[rk]);
  return 1;
}

int ldf_11( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t const8 )
{
  uint8_t *b = (uint8_t*) CAST_INT reg[rj];
  CHECK_READ_ANNOTS( proc_id, b );
  reg[ri] = *(uint64_t*)(float*)(b + const8);
  return 1;
}

int ldd_12( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t rk )
{
  uint8_t *b = (uint8_t*) CAST_INT reg[rj];
  CHECK_READ_ANNOTS( proc_id, b );
  reg[ri] = *(uint64_t*)(double*)(b + reg[rk]);
  //block *b = (block*) CAST_INT reg[rj];
  //reg[ri] = *(int64_t *)(b->data + const8);
  return 1;
}

int ldd_13( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t const8 )
{
  uint8_t *b = (uint8_t*) CAST_INT reg[rj];
  CHECK_READ_ANNOTS( proc_id, b );
  reg[ri] = *(uint64_t*)(double*)(b + const8);
  //block *b = (block*) CAST_INT reg[rj];
  //reg[ri] = *(int64_t *)(b->data + const8);
  return 1;
}

int ldimm_14( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t c3, uint8_t c4 )
{
  uint16_t const16  = TWO_8_TO_16( c3, c4 );

#if TRACK_EXEC
  fprintf( stderr, "ldimm_14: const16: %d\n", const16 );
#endif

  reg[ri] = (uint64_t)const16;

#if TRACK_EXEC
  fprintf( stderr, "ldimm_14: reg[ri]: %lld\n", reg[ri] );
#endif

  return 1;
}

int ldimm2_15( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t c3, uint8_t c4 )
{
  uint16_t const16  = TWO_8_TO_16( c3, c4 );
  reg[ri] <<= 16;
  reg[ri] = reg[ri] | (uint64_t)const16;
#if TRACK_EXEC
  fprintf( stderr, "ldimm_14: reg[ri]: %lld\n", reg[ri] );
#endif
  return 1;
}

int stb_16( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t rk )
{
  uint8_t *b = (uint8_t*) CAST_INT reg[rj];
  if( reg[rk] > BLOCK_LENGTH( b ) )
    EXIT_WITH_ERROR("Error: Bad memory access in stb_17!\n"
                    "This should actually throw an exception!\n");
  CHECK_WRITE_ANNOTS( proc_id, b );
  *(uint8_t *)(b + reg[rk]) = reg[ri];
  return 1;
}

int stb_17( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t const8 )
{
  uint8_t *b = (uint8_t*) CAST_INT reg[rj];
  if( const8 > BLOCK_LENGTH( b ) )
    EXIT_WITH_ERROR("Error: Bad memory access in stb_17!\n"
                    "This should actually throw an exception!\n");
  CHECK_WRITE_ANNOTS( proc_id, b );
  *(uint8_t *)(b + const8) = reg[ri];
  return 1;
}

int sts_18( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t rk )
{
  uint8_t *b = (uint8_t*) CAST_INT reg[rj];
  if( reg[rk] > BLOCK_LENGTH( b ) )
    EXIT_WITH_ERROR("Error: Bad memory access in sts_18!\n"
                    "This should actually throw an exception!\n");
  CHECK_WRITE_ANNOTS( proc_id, b );
  *(int8_t *)(b + reg[rk]) = reg[ri];
  return 1;
}

int sts_19( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t const8 )
{
  uint8_t *b = (uint8_t*) CAST_INT reg[rj];
  if( const8 > BLOCK_LENGTH( b ) )
    EXIT_WITH_ERROR("Error: Bad memory access in sts_18!\n"
                    "This should actually throw an exception!\n");
  CHECK_WRITE_ANNOTS( proc_id, b );
  *(int8_t *)(b + const8) = reg[ri];
  return 1;
}

int sti_20( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t rk )
{
  uint8_t *b = (uint8_t*) CAST_INT reg[rj];
  if( reg[rk] > BLOCK_LENGTH( b ) )
    EXIT_WITH_ERROR("Error: Bad memory access in sti_21!\n"
                    "This should actually throw an exception!\n");
  CHECK_WRITE_ANNOTS( proc_id, b );
  *(int32_t *)(b + reg[rk]) = reg[ri];
  return 1;
}

int sti_21( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t const8 )
{
  uint8_t *b = (uint8_t*) CAST_INT reg[rj];
  if( const8 > BLOCK_LENGTH( b ) )
    EXIT_WITH_ERROR("Error: Bad memory access in sti_21!\n"
                    "This should actually throw an exception!\n");
  CHECK_WRITE_ANNOTS( proc_id, b );
  *(int32_t *)(b + const8) = reg[ri];

  return 1;
}

int stl_22( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t rk )
{
  uint8_t *b = (uint8_t*) CAST_INT reg[rj];
  if( reg[rk] > BLOCK_LENGTH( b ) )
    EXIT_WITH_ERROR("Error: Bad memory access in stl_22!\n"
                    "This should actually throw an exception!\n");
  CHECK_WRITE_ANNOTS( proc_id, b );
  *(int64_t *)(b + reg[rk]) = reg[ri];

  return 1;
}

int stl_23( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t const8 )
{
  uint8_t *b = (uint8_t*) CAST_INT reg[rj];
  if( const8 > BLOCK_LENGTH( b ) )
    EXIT_WITH_ERROR("Error: Bad memory access in stl_23!\n"
                    "This should actually throw an exception!\n");
  CHECK_WRITE_ANNOTS( proc_id, b );
  *(int64_t *)(b + const8) = reg[ri];

  return 1;
}

int stf_24( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t rk )
{
  uint8_t *b = (uint8_t*) CAST_INT reg[rj];
  if( reg[rk] > BLOCK_LENGTH( b ) )
    EXIT_WITH_ERROR("Error: Bad memory access in stf_24!\n"
                    "This should actually throw an exception!\n");
  CHECK_WRITE_ANNOTS( proc_id, b );
  *(uint32_t *)(b + reg[rk]) = *(uint32_t*) (float*) &reg[ri];

  return 1;
}

int stf_25( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t const8 )
{
  uint8_t *b = (uint8_t*) CAST_INT reg[rj];
  if( const8 > BLOCK_LENGTH( b ) )
    EXIT_WITH_ERROR("Error: Bad memory access in stf_25!\n"
                    "This should actually throw an exception!\n");
  CHECK_WRITE_ANNOTS( proc_id, b );
  *(uint32_t *)(b + const8) = *(uint32_t*) (float*) &reg[ri];

  return 1;
}

int std_26( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t rk )
{
  uint8_t *b = (uint8_t*) CAST_INT reg[rj];
  if( reg[rk] > BLOCK_LENGTH( b ) )
    EXIT_WITH_ERROR("Error: Bad memory access in std_26!\n"
                    "This should actually throw an exception!\n");
  CHECK_WRITE_ANNOTS( proc_id, b );
  *(uint64_t *)(b + reg[rk]) = *(uint64_t*) (double*) &reg[ri];

  return 1;
}

int std_27( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t const8 )
{
  uint8_t *b = (uint8_t*) CAST_INT reg[rj];
  if( const8 > BLOCK_LENGTH( b ) )
    EXIT_WITH_ERROR("Error: Bad memory access in std_27!\n"
                    "This should actually throw an exception!\n");
  CHECK_WRITE_ANNOTS( proc_id, b );
  *(uint64_t *)(b + const8) = *(uint64_t*) (double*) &reg[ri];

  return 1;
}

int ldblkid_28( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
                uint8_t opcode, uint8_t ri, uint8_t c3, uint8_t c4 )
{
  uint16_t const16 = TWO_8_TO_16( c3, c4 );
  /*FIXME: Check index against f_block count */
  uint8_t *b = (uint8_t*) CAST_INT 
               ((uint64_t*) CAST_INT reg[BLOCK_REG])[const16];
#if TRACK_EXEC
  fprintf( stderr, "\tldblkid: b: %p\n", b );
#endif
  reg[ri] = (uint64_t) CAST_INT b;
  return 1;
}

int ldnative_29( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
                uint8_t opcode, uint8_t ri, uint8_t c3, uint8_t c4 )
{
  uint16_t const16 = TWO_8_TO_16( c3, c4 );
  /*FIXME: Check index against f_block count */
  uint8_t *b = (uint8_t*) CAST_INT 
               ((uint64_t*) CAST_INT reg[BLOCK_REG])[const16];
#if TRACK_EXEC
  fprintf( stderr, "\tldnative: b: %p\n", b );
#endif
  reg[ri] = (uint64_t) CAST_INT b;
  return 1;
}

int addl_32( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
              uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t rk )
{
  reg[ri] = (long)reg[rj] + (long)reg[rk];
  return 1;
}

int addl_33( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
              uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t const8 )
{
  reg[ri] = (long)reg[rj] + (long)const8;
#if TRACK_EXEC
  fprintf( stderr, "reg[ri]: %d\n", (int)reg[ri] );
#endif
  return 1;
}

int subl_34( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t rk )
{
  reg[ri] = (long)reg[rj] - (long)reg[rk];
  return 1;
}

int subl_35( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t const8 )
{
  reg[ri] = (long)reg[rj] - (long)const8;
  return 1;
}

int mull_36( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t rk )
{
  reg[ri] = (long)reg[rj] * (long)reg[rk];
  return 1;
}

int mull_37( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t const8 )
{
  reg[ri] = (long)reg[rj] * (long)const8;
  return 1;
}

int divl_38( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t rk )
{
  reg[ri] = (long)reg[rj] / (long)reg[rk];
  return 1;
}

int divl_39( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t const8 )
{
  reg[ri] = (long)reg[rj] / (long)const8;
  return 1;
}

int reml_40( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t rk )
{
  reg[ri] = (long)reg[rj] % (long)reg[rk];
#if TRACK_EXEC
  fprintf( stderr, "::\t%ld, %ld, %ld\n", reg[ri], (long)reg[rj], (long)reg[rk]);
#endif
  return 1;
}

int reml_41( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t const8 )
{
  reg[ri] = (long)reg[rj] % (long)const8;
#if TRACK_EXEC
  fprintf( stderr, "::\t%ld, %ld, %ld\n", reg[ri], (long)reg[rj], (long)const8);
#endif
  return 1;
}

int negl_42( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t rk )
{
  reg[ri] = -(*(long*)&reg[rj]);
  return 1;
}

int addd_43( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t rk )
{
  double augend, addend, sum;

  augend = *(double*) &reg[rj];
  addend = *(double*) &reg[rk];
  sum = augend + addend;

#if TRACK_EXEC
  fprintf( stderr, "augend: %f\naddend: %f\nsum: %f\n",
                   augend, addend, sum );
#endif

  reg[ri] = *(uint64_t*) &sum;

#if TRACK_EXEC
  fprintf( stderr, "reg[ri]: %f\n", *(double*) &reg[ri]);
#endif

  return 1;
}

int subd_44( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t rk )
{
  double m, n, r;
  m = *(double*)&reg[rj];
  n = *(double*)&reg[rk];
  r = m - n;
  reg[ri] = *(uint64_t*)&r;
  return 1;
}

int muld_45( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t rk )
{
  double m, n, r;

  m = *(double*) &reg[rj];
  n = *(double*) &reg[rk];
  r = m * n;

#if TRACK_EXEC
  fprintf( stderr, "m: %f\nn: %f\nr: %f\n",
                   m, n, r );
#endif
  reg[ri] = *(uint64_t*) &r;
#if TRACK_EXEC
  fprintf( stderr, "reg[ri]: %f\n", *(double*) &reg[ri] );
#endif

  return 1;
}

int divd_46( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t rk )
{
  double dividend, divisor, quotient;

  dividend = *(double*) &reg[rj];
  divisor  = *(double*) &reg[rk];

  if( divisor == 0. )
    EXIT_WITH_ERROR("Divide by zero!!!!!!\n");

  quotient = dividend / divisor;
#if TRACK_EXEC
  fprintf( stderr, "dividend: %f\ndivisor: %f\nquotient: %f\n",
                   dividend, divisor, quotient );
#endif

  reg[ri] = *(uint64_t*) &quotient;
#if TRACK_EXEC
  fprintf( stderr, "reg[ri]: %f\n", *(double*) &reg[ri] );
#endif

  return 1;
}

int negd_47( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t rk )
{
  double n;
  n = -(*(double*)&reg[rj]);
  reg[ri] = *(uint64_t*)&n;
  return 1;
}

int cvtld_48( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t c4 )
{
  double d;
  d = (double) reg[rj];
  reg[ri] =  *(uint64_t*)&d;
#if TRACK_EXEC
  fprintf( stderr, "cvtld: reg[rj]: %d\n", reg[rj]);
  fprintf( stderr, "cvtld: reg[ri]: %f\n", *(double*)&reg[ri]);
#endif
  return 1;
}

int cvtdl_49( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t c4 )
{
  long l;
  l = (long)*(double*)&reg[rj];
  reg[ri] = *(uint64_t*)&l;
#if TRACK_EXEC
  fprintf( stderr, "cvtdl: reg[rj]: %f\n", *(double*)&reg[rj]);
  fprintf( stderr, "cvtdl: reg[ri]: %d\n", reg[ri]);
#endif
  return 1;
}

/* FIXME: Can you do negative shifts? */
int lshift_50( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t rk )
{
  uint64_t x, r;
  uint64_t s;
  x = reg[rj];
  s = reg[rk];
  r = x << (s % 64);
  reg[ri] = r;
  return 1;
}

int lshift_51( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t const8 )
{
  uint64_t x, r;
  x = reg[rj];
  r = x << (const8 % 64);
  reg[ri] = r;
  return 1;
}

int rshift_52( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t rk )
{
  int64_t x, r;
  uint64_t s;
  x = reg[rj];
  s = reg[rk];
  r = x >> (s % 64);
  reg[ri] = r;
  return 1;
}

int rshift_53( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t const8 )
{
  int64_t x, r;
  x = reg[rj];
  r = x >> (const8 % 64);
  reg[ri] = r;
  return 1;
}

int rshiftu_54( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t rk )
{
  uint64_t x, r;
  uint64_t s;
  x = reg[rj];
  s = reg[rk];
  r = x >> (s % 64);
  reg[ri] = r;
  return 1;
}

int rshiftu_55( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t const8 )
{
  uint64_t x, r;
  x = reg[rj];
  r = x >> (const8 % 64);
  reg[ri] = r;
  return 1;
}

int and_56( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t rk )
{
  reg[ri] = reg[rj] & reg[rk];
  return 1;
}

int or_57( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t rk )
{
  reg[ri] = reg[rj] | reg[rk];
  return 1;
}

int xor_58( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t rk )
{
  reg[ri] = reg[rj] ^ reg[rk];
  return 1;
}

int ornot_59( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t rk )
{
  reg[ri] = reg[rj] | (~reg[rk]);
  return 1;
}

int cmpeq_64( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t rk )
{
  int64_t x, y;
  x = reg[rj];
  y = reg[rk];
  reg[ri] = (x == y);
  return 1;
}

int cmpeq_65( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t const8 )
{
  int64_t x, y;
  x = reg[rj];
  y = (int64_t)(int8_t)const8;
  reg[ri] = (x == y);
  return 1;
}

int cmple_66( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t rk )
{
  int64_t x, y;
  x = reg[rj];
  y = reg[rk];
  reg[ri] = (x <= y);
  return 1;
}

int cmple_67( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t const8 )
{
  int64_t x, y;
  x = reg[rj];
  y = (int64_t)(int8_t)const8;
  reg[ri] = (x <= y);
  return 1;
}

int cmplt_68( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t rk )
{
  int64_t x, y;
  x = reg[rj];
  y = reg[rk];
  reg[ri] = (x < y);
  return 1;
}

int cmplt_69( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t const8 )
{
  int64_t x, y;
  x = reg[rj];
  y = (int64_t)(int8_t)const8;
  reg[ri] = (x < y);
  return 1;
}

int cmpule_70( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t rk )
{
  uint64_t x, y;
  x = reg[rj];
  y = reg[rk];
  reg[ri] = (x <= y);
  return 1;
}

int cmpule_71( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t const8 )
{
  uint64_t x, y;
  x = reg[rj];
  y = (uint64_t)const8;
  reg[ri] = (x <= y);
  return 1;
}

int cmpult_72( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t rk )
{
  uint64_t x, y;
  x = reg[rj];
  y = reg[rk];
  reg[ri] = (x < y);
  return 1;
}

int cmpult_73( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t const8 )
{
  uint64_t x, y;
  x = reg[rj];
  y = (uint64_t)const8;
  reg[ri] = (x < y);
  return 1;
}

int fcmpeq_74( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t rk )
{
  double x, y;
  x = *(double*)&reg[rj];
  y = *(double*)&reg[rk];
  reg[ri] = (x == y);
  return 1;
}

int fcmple_75( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t rk )
{
  double x, y;
  x = *(double*)&reg[rj];
  y = *(double*)&reg[rk];
  reg[ri] = (x <= y);
  return 1;
}

int fcmplt_76( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t rk )
{
  double x, y;
  x = *(double*)&reg[rj];
  y = *(double*)&reg[rk];
  reg[ri] = (x < y);
  return 1;
}

int jmp_80( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  uint16_t uconst16 = TWO_8_TO_16( c3, c4 );
  int16_t const16 = *(int16_t*) &uconst16;
  /* FIXME */
  CIO += const16 * 4;
  return 1;
}

int jmp_81( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t rk )
{
  CIO = CIO + reg[rj] + (reg[rk] * 4);
  return 1;
}

int btrue_82( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t c3, uint8_t c4 )
{
  uint16_t uconst16;
  int16_t const16;
  if( reg[ri] )
  {
    uconst16 = TWO_8_TO_16( c3, c4 );
    const16 = *(int16_t*) &uconst16;
    /* FIXME */
    CIO += const16 * 4;
  }
  return 1;
}

int bfalse_83( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t c3, uint8_t c4 )
{  
  uint16_t uconst16;
  int16_t const16;
  if( !reg[ri] )
  {
    uconst16 = TWO_8_TO_16( c3, c4 );
    const16 = *(int16_t*) &uconst16;
    /* FIXME */
    CIO += const16 * 4;
  }

  return 1;
}

/* FIXME: Hacked together functions and macros to make malloc work.
 * Clean this up at some point.
 */
int valid_bid( uint64_t id )
{
  return 1;
}

int alloc_blk_96( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t rk )
{
  uint8_t *b;

  pthread_mutex_lock( &malloc_xpvm_mu );

  b = (uint8_t*)malloc_xpvm( reg[rj] );
  BLOCK_LENGTH( b ) = reg[rj];
  if( ! reg[rk] )
    SET_BLOCK_OWNED( b );
  else
    SET_BLOCK_CHAINED( b, reg[rk] );
  BLOCK_OWNER( b ) = proc_id;
  reg[ri] = (uint64_t)(uint64_t*) b;

  pthread_mutex_unlock( &malloc_xpvm_mu );

  return 1;
}

int alloc_private_blk_97( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t rk )
{
  uint8_t *b;

  pthread_mutex_lock( &malloc_xpvm_mu );

  b = (uint8_t*)malloc_xpvm( reg[rj] );

  BLOCK_LENGTH( b ) = reg[rj];
  SET_BLOCK_OWNED( b );
  SET_BLOCK_PRIVATE( b );
  BLOCK_OWNER( b ) = proc_id;
  reg[ri] = (uint64_t)(uint64_t*) b;

  pthread_mutex_unlock( &malloc_xpvm_mu );

  return 1;
}

int aquire_blk_98( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t rk )
{
  uint8_t *b = (uint8_t*) reg[rj];
  CHECK_AQUIRE_ANNOTS( proc_id, b );
  if( CHECK_OWNED(b) )
  {
    reg[rk] = 0;
    return 1;
  }
  /* reg[rk] is set in this macro */
  SET_BLOCK_OWNED(b);
  CMPXCHG( &(BLOCK_OWNER(b)), NULL, proc_id );
  return 1;
}

int release_blk_99( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t rk )
{
  uint8_t *b = (uint8_t*) reg[rj];

  CHECK_RELEASE_ANNOTS( proc_id, b );
  SET_BLOCK_FREE(b);
  BLOCK_OWNER(b) = 0;
  return 1;
}

int dtraits_100( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int rannots_101( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int towner_102( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int lock_103( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int unlock_104( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int wait_105( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int sig_106( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int sigall_107( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int ldfunc_112( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
                uint8_t opcode, uint8_t ri, uint8_t c3, uint8_t c4 )
{
  uint16_t const16 = TWO_8_TO_16( c3, c4 );
  /*FIXME: Check index against f_block count */
  uint8_t *b = (uint8_t*) CAST_INT 
               ((uint64_t*) CAST_INT reg[BLOCK_REG])[const16];
#if TRACK_EXEC
  fprintf( stderr, "\tldfunc: b: %p\n", b );
#endif
  reg[ri] = (uint64_t) CAST_INT b;
  return 1;
}

int ldfunc_113( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int call_114( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t const8 )
{
  uint8_t *b = (uint8_t*) CAST_INT reg[rj];
#if TRACK_EXEC
  fprintf( stderr, "\tcall: b: %p\n", b );
  fprintf( stderr, "INST_MASK: %lld\n", (uint64_t) INST_MASK );
#endif
  CHECK_EXEC_ANNOTS( proc_id, b );
  stack_frame *f = calloc( 1, sizeof(stack_frame) );
  /* FIXME: Throw OutOfMemory Exception */
  if( !f )
    EXIT_WITH_ERROR("Error: malloc failed in call_114\n");
  f->pc = reg[PC_REG];
  f->cio = CIO;
  f->cib = CIB;
  f->reg255 = reg[255];
  f->ret_reg = ri;

  uint32_t frame_size = BLOCK_FRAME_SIZE( b );
#if TRACK_EXEC
  fprintf( stderr, "\tcall_114: frame_size: %d\n", frame_size );
#endif
  if( frame_size )
  {
    f->block = calloc( frame_size + BLOCK_HEADER_LENGTH, sizeof(uint8_t) );
    /* FIXME: Throw OutOfMemory Exception */
    if( !f->block )
      EXIT_WITH_ERROR("Error: malloc failed in call_114\n");

    f->block = f->block + BLOCK_HEADER_LENGTH;
    BLOCK_LENGTH( f->block ) = frame_size;

  }
  f->prev = *stack;
  *stack = f;
  reg[STACK_FRAME_REG] = (uint64_t) CAST_INT f->block;
  reg[PC_REG] = (uint64_t) CAST_INT b;
  CIB = (uint64_t) CAST_INT b;
  CIO = 0;

  return 1;
}

/* FIXME:
 * This is wrong. The user first needs to load the native function into
 * a register then pass the actual function pointer to this opcode
 * and then it is called.
 * FIXME:
 */
int calln_115( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t const8 )
{
  char *name = native_funcs[reg[rj]];
  char *error = NULL;
  int i = 0;
  /*int (*fp)( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
                     uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 );*/
  int (*fp)( void );
#if TRACK_EXEC
  fprintf( stderr, "name: %s\n", name );
  fprintf( stderr, "num args: %d\n", const8 );
#endif

  fp = (int (*)(void)) dlsym( __lh, name );
  if( (error = dlerror()) != NULL )
    EXIT_WITH_ERROR("Error: dlsym failed in calln_115\n");

  #if 1
  for( i = 0; i < const8 && i < 10; i++ )
  {
    uint64_t to_push = reg[i+1];
    switch( i )
    {
      case 0:
        __asm__("movq %0, %%rdi\t\n"
                : /* no output registers */
                : "r" (to_push) /* input register read only */
                : "%rdi" /* clobbered registers */
                );
      break;
      case 1:
        __asm__("movq %0, %%rsi\t\n"
                :
                : "r" (to_push) 
                : "%rsi"
                );
      break;
      case 2:
        __asm__("movq %0, %%rdx\t\n"
                :
                : "r" (to_push) 
                : "%rdx"
                );
      break;
      case 3:
        __asm__("movq %0, %%rcx\t\n"
                : 
                : "r" (to_push)
                : "%rcx"
                );
      break;
      case 4:
        __asm__("movq %0, %%r8\t\n"
                : 
                : "r" (to_push) 
                : "%r8"
                );
      break;
      case 5:
        __asm__("movq %0, %%r9\t\n"
                : 
                : "r" (to_push)
                : "%r9"
                );
      break;
      default:
        __asm__("pushq %0\t\n"
                : /* no output registers */
                : "r" (to_push) /* input register read only */
                : "%rsp" /* Do I need to specify rsp since I did a push? */
                );
      break;
    }
  }
  i = (*fp)();
  for( i = 0; i < const8 && i < 10; i++ )
  {
    __asm__("popq %%rax\t\n"
            : /* no output registers */
            : /* no input registers */
            : "%rax" /* Do I need to specify esp since I did a push? */
            );
  }
  #else
  i = (*fp)( proc_id, reg, stack, opcode, ri, rj, const8 );
  #endif
  reg[ri] = i;
  if( !i )
    return 0;

  return 1;
}

int ret_116( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t c2, uint8_t rj, uint8_t c4 )
{
#if TRACK_EXEC > 1
  fprintf( stderr, "\treg: %p\n", reg );
  fprintf( stderr, "\tstack: %p\n", *stack );
#endif
  reg[(*stack)->ret_reg] = reg[rj];
  reg[PC_REG] = (*stack)->pc;
  reg[255] = (*stack)->reg255;
  CIO = (*stack)->cio;
  CIB = (*stack)->cib;
  /* pop frame */
#if TRACK_EXEC
  fprintf( stderr, "\tret_reg: %lld\n", (*stack)->ret_reg );
  fprintf( stderr, "\treg[ret_reg]: %lld\n", reg[(*stack)->ret_reg] );
#endif
  /* popped last stack, halt */
  if( !(*stack)->prev )
    return 0;
  stack_frame *old_frame = *stack;
  *stack = (*stack)->prev;
  if( old_frame->block )
    free( old_frame->block - BLOCK_HEADER_LENGTH );
  free( old_frame );

  return 1;
}

int throw_128( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int retrieve_129( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}

int init_proc_144( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t const8 )
{
  int i = 0;
  uint64_t *args = calloc( const8+1, sizeof(uint64_t) );
  if( !args )
    EXIT_WITH_ERROR("Error: malloc failed in init_proc_144\n");

  for( i = 0; i <= const8; i++  )
    args[i] = reg[i];

  do_init_proc( &reg[ri], reg[rj], const8, args );
  return 1;
}

int join_145( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t c4 )
{
  do_proc_join( reg[ri], &reg[rj] );
  return 1;
}

int join2_146( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t opcode, uint8_t ri, uint8_t rj, uint8_t c4 )
{
  do_join2( reg[ri], &reg[rj] );
  return 1;
}

int whoami_147( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
            uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 )
{
  return 1;
}
