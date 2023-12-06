#ifndef moca_assembler_bytecode_h
#define moca_assembler_bytecode_h

#define mov_opcode      (usint8)0xB

#define r_ax_opcode     (usint8)0x8
#define r_ah_opcode     (usint8)(r_ax_opcode >> 1) & 0xFF
#define r_al_opcode     (usint8)(r_ah_opcode >> 4) & 0xFF

#endif