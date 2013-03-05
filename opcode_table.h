/*
 * opcode_table.h
 *
 * Table of opcodes for the XPVM.
 *
 * Author: Jeffrey Picard
 */
#ifndef _OPCODE_TABLE_H_
#define _OPCODE_TALBE_H_

/* 
 * Table of opcodes.
 *    Indexed by opcode number.
 *    Contains opcode name as a string, format type 
 *    as an int and a function pointer to the C 
 *    implementation of the opcode.
 */
#define MAX_OPCODE_XPVM 150
static struct opcode_info
{
  char* opcode;
  int format;
  int (*formatFunc)( unsigned int proc_id, uint64_t *reg, stack_frame **stack,
                     uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4 );
}
opcodes[] =
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
{"ldblkid",   0, ldblkid_28},        /* 28 */
{"29",        0, NULL},        /* 29 */
{"30",        0, NULL},        /* 30 */
{"31",        0, NULL},        /* 31 */
{"addl",      1, addl_32},     /* 32 */
{"addl",      2, addl_33}, /* 33 */
{"subl",      1, subl_34},     /* 34 */
{"subl",      2, NULL}, /* 35 */
{"mull",      1, mull_36},     /* 36 */
{"mull",      2, NULL}, /* 37 */
{"divl",      1, NULL}, /* 38 */
{"divl",      2, NULL}, /* 39 */
{"reml",      1, NULL}, /* 40 */
{"reml",      2, NULL}, /* 41 */
{"negl",      1, NULL}, /* 42 */
{"addd",      1, addd_43}, /* 43 */
{"subd",      1, NULL}, /* 44 */
{"muld",      1, muld_45}, /* 45 */
{"divd",      1, divd_46}, /* 46 */
{"negd",      1, NULL}, /* 47 */
{"cvtld",     1, cvtld_48}, /* 48 */
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
{"cmplt",     1, cmplt_68}, /* 68 */
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
{"jmp",       3, jmp_80}, /* 80 */
{"jmp",       1, NULL}, /* 81 */
{"btrue",     3, btrue_82}, /* 82 */
{"bfalse",    3, bfalse_83}, /* 83 */
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
{"calln",     2, calln_115},        /* 115 */
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
{"initProc",  2, initProc_144}, /* 144 */
{"join",      1, join_145}, /* 145 */
{"join2",     1, NULL}, /* 146 */
{"whoami",    1, NULL}, /* 147 */
};

#endif
