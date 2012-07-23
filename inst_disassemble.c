/*
 * inst_disassemble.c
 *
 * This program tests the process for disassembling the 32 bit
 * instructions for the XPVM into their component parts.
 *
 * Author: Jeffrey Picard (jpicardnh@gmail.com)
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define INST1F1 0xAABBCCDD

void format1_dis( uint32_t );
void format2_dis( uint32_t );
void format3_dis( uint32_t );

int main( int argc, char **argv )
{
  format1_dis( INST1F1 );
  format3_dis( INST1F1 );

  return 0;
}

void format1_dis( uint32_t inst )
{
  uint8_t opcode = (inst & 0xFF000000) >> 24;
  uint8_t regi   = (inst & 0x00FF0000) >> 16;
  uint8_t regj   = (inst & 0x0000FF00) >> 8;
  uint8_t regk   = inst & 0x000000FF;

  fprintf( stderr, 
    "inst: %x\n"
    "opcode: %x\n" 
    "regi: %x\n" 
    "regj: %x\n" 
    "regk: %x\n\n",
    inst, opcode, regi, regj, regk
    );
}

void format2_dis( uint32_t inst )
{
  /* Formats 1 and 2 disassemble in the same way. */
  format1_dis( inst );
}

void format3_dis( uint32_t inst )
{
  uint8_t  opcode  = (inst & 0xFF000000) >> 24;
  uint8_t  regi    = (inst & 0x00FF0000) >> 16;
  uint16_t const16 = (inst & 0x0000FFFF);

  fprintf( stderr, 
    "inst: %x\n"
    "opcode: %x\n" 
    "regi: %x\n" 
    "const16: %x\n\n",
    inst, opcode, regi, const16
    );
}
