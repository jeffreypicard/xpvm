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
{"0",                     0, NULL},        /* 0 */
{"1",                     0, NULL},        /* 1 */
{"ldb",                   0, ldb_2},       /* 2 */
{"ldb",                   0, ldb_3},       /* 3 */
{"lds",                   0, lds_4},       /* 4 */
{"lds",                   0, lds_5},       /* 5 */
{"ldi",                   0, ldi_6},       /* 6 */
{"ldi",                   0, ldi_7},       /* 7 */
{"ldl",                   0, ldl_8},       /* 8 */
{"ldl",                   0, ldl_9},       /* 9 */
{"ldf",                   0, NULL}, /* 10 */
{"ldf",                   0, NULL}, /* 11 */
{"ldd",                   0, NULL}, /* 12 */
{"ldd",                   0, NULL}, /* 13 */
{"ldimm",                 0, ldimm_14},    /* 14 */
{"ldimm2",                0, ldimm2_15},   /* 15 */
{"stb",                   0, stb_16}, /* 16 */
{"stb",                   0, stb_17}, /* 17 */
{"sts",                   0, NULL}, /* 18 */
{"sts",                   0, NULL}, /* 19 */
{"sti",                   0, NULL}, /* 20 */
{"sti",                   0, sti_21},      /* 21 */
{"stl",                   0, NULL}, /* 22 */
{"stl",                   0, NULL}, /* 23 */
{"stf",                   0, NULL}, /* 24 */
{"stf",                   0, NULL}, /* 25 */
{"std",                   0, NULL}, /* 26 */
{"std",                   0, NULL}, /* 27 */
{"ldblkid",               0, ldblkid_28},   /* 28 */
{"29",                    0, NULL},        /* 29 */
{"30",                    0, NULL},        /* 30 */
{"31",                    0, NULL},        /* 31 */
{"addl",                  0, addl_32},      /* 32 */
{"addl",                  0, addl_33},      /* 33 */
{"subl",                  0, subl_34},      /* 34 */
{"subl",                  0, subl_35},      /* 35 */
{"mull",                  0, mull_36},      /* 36 */
{"mull",                  0, mull_37},      /* 37 */
{"divl",                  0, divl_38}, /* 38 */
{"divl",                  0, divl_39}, /* 39 */
{"reml",                  0, reml_40}, /* 40 */
{"reml",                  0, reml_41}, /* 41 */
{"negl",                  0, negl_42}, /* 42 */
{"addd",                  0, addd_43}, /* 43 */
{"subd",                  0, subd_44}, /* 44 */
{"muld",                  0, muld_45}, /* 45 */
{"divd",                  0, divd_46}, /* 46 */
{"negd",                  0, negd_47}, /* 47 */
{"cvtld",                 0, cvtld_48}, /* 48 */
{"cvtdl",                 0, cvtdl_49}, /* 49 */
{"lshift",                0, lshift_50}, /* 50 */
{"lshift",                0, lshift_51}, /* 51 */
{"rshift",                0, rshift_52}, /* 52 */
{"rshift",                0, rshift_53}, /* 53 */
{"rshiftu",               0, rshiftu_54}, /* 54 */
{"rshiftu",               0, rshiftu_55}, /* 55 */
{"and",                   0, and_56}, /* 56 */
{"or",                    0, or_57}, /* 57 */
{"xor",                   0, xor_58}, /* 58 */
{"ornot",                 0, ornot_59}, /* 59 */
{"60",                    0, NULL},        /* 60 */
{"61",                    0, NULL},        /* 61 */
{"62",                    0, NULL},        /* 62 */
{"63",                    0, NULL},        /* 63 */
{"cmpeq",                 0, NULL}, /* 64 */
{"cmpeq",                 0, NULL}, /* 65 */
{"cmple",                 0, NULL}, /* 66 */
{"cmple",                 0, NULL}, /* 67 */
{"cmplt",                 0, cmplt_68}, /* 68 */
{"cmplt",                 0, NULL}, /* 69 */
{"cmpule",                0, NULL}, /* 70 */
{"cmpule",                0, NULL}, /* 71 */
{"cmpult",                0, NULL}, /* 72 */
{"cmpult",                0, NULL}, /* 73 */
{"fcmpeq",                0, NULL}, /* 74 */
{"fcmple",                0, NULL}, /* 75 */
{"fcmplt",                0, NULL}, /* 76 */
{"77",                    0, NULL},        /* 77 */
{"78",                    0, NULL},        /* 78 */
{"79",                    0, NULL},        /* 79 */
{"jmp",                               0, jmp_80}, /* 80 */
{"jmp",                   0, NULL}, /* 81 */
{"btrue",                 0, btrue_82}, /* 82 */
{"bfalse",                0, bfalse_83}, /* 83 */
{"84",                    0, NULL},        /* 84 */
{"85",                    0, NULL},        /* 85 */
{"86",                    0, NULL},        /* 86 */
{"87",                    0, NULL},        /* 87 */
{"88",                    0, NULL},        /* 88 */
{"89",                    0, NULL},        /* 89 */
{"90",                    0, NULL},        /* 90 */
{"91",                    0, NULL},        /* 91 */
{"92",                    0, NULL},        /* 92 */
{"93",                    0, NULL},        /* 93 */
{"94",                    0, NULL},        /* 94 */
{"95",                    0, NULL},        /* 95 */
{"alloc_blk",             0, alloc_blk_96}, /* 96 */
{"alloc_private_blk",     0, alloc_private_blk_97}, /* 97 */
{"aquire_blk",            0, aquire_blk_98}, /* 98 */
{"release_blk",           0, release_blk_99}, /* 99 */
{"dtraits",               0, NULL}, /* 100 */
{"rannots",               0, NULL}, /* 101 */
{"towner",                0, NULL}, /* 102 */
{"lock",                  0, NULL}, /* 103 */
{"unlock",                0, NULL}, /* 104 */
{"wait",                  0, NULL}, /* 105 */
{"sig",                   0, NULL}, /* 106 */
{"sigall",                0, NULL}, /* 107 */
{"108",                   0, NULL},        /* 108 */
{"109",                   0, NULL},        /* 109 */
{"110",                   0, NULL},        /* 110 */
{"111",                   0, NULL},        /* 111 */
{"ldfunc",                0, ldfunc_112},  /* 112 */
{"ldfunc",                0, NULL}, /* 113 */
{"call",                  0, call_114},    /* 114 */
{"calln",                 0, calln_115},        /* 115 */
{"ret",                   0, ret_116},     /* 116 */
{"117",                   0, NULL},        /* 117 */
{"118",                   0, NULL},        /* 118 */
{"119",                   0, NULL},        /* 119 */
{"120",                   0, NULL},        /* 120 */
{"121",                   0, NULL},        /* 121 */
{"122",                   0, NULL},        /* 122 */
{"123",                   0, NULL},        /* 123 */
{"124",                   0, NULL},        /* 124 */
{"125",                   0, NULL},        /* 125 */
{"126",                   0, NULL},        /* 126 */
{"127",                   0, NULL},        /* 127 */
{"throw",                 0, NULL}, /* 128 */
{"retrieve",              0, NULL}, /* 129 */
{"130",                   0, NULL},        /* 130 */
{"131",                   0, NULL},        /* 131 */
{"132",                   0, NULL},        /* 132 */
{"133",                   0, NULL},        /* 133 */
{"134",                   0, NULL},        /* 134 */
{"135",                   0, NULL},        /* 135 */
{"136",                   0, NULL},        /* 136 */
{"137",                   0, NULL},        /* 137 */
{"138",                   0, NULL},        /* 138 */
{"139",                   0, NULL},        /* 139 */
{"140",                   0, NULL},        /* 140 */
{"141",                   0, NULL},        /* 141 */
{"142",                   0, NULL},        /* 142 */
{"143",                   0, NULL},        /* 143 */
{"init_proc",             0, init_proc_144}, /* 144 */
{"join",                  0, join_145}, /* 145 */
{"join2",                 0, NULL}, /* 146 */
{"whoami",                0, NULL}, /* 147 */
};

#endif
