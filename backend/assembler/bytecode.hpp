#ifndef moca_assembler_bytecode_h
#define moca_assembler_bytecode_h

#define mov_opcode      (usint8)0xB

#define r_ax_opcode     (usint8)0x8
#define r_ah_opcode     (usint8)(r_ax_opcode >> 1) & 0xFF
#define r_al_opcode     (usint8)(r_ax_opcode >> 4) & 0xFF

enum class RegularMov
{
    mov_ax      = (mov_opcode << 4) | r_ax_opcode,
    mov_ah      = (mov_opcode << 4) | r_ah_opcode,
    mov_al      = (mov_opcode << 4) | r_al_opcode
};

/* TODO: Probably remove this and approach this a different way. For now, this works. */
constexpr int8 reg_mov_instructions[] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    (int8)RegularMov::mov_ax, (int8)RegularMov::mov_ah, (int8)RegularMov::mov_al
};

enum class MovTypes
{
    regular_mov
};

#ifdef __cplusplus
extern "C"
{
#endif

struct bit16_opcode_info
{
    struct {
        usint8          opcode;

        /* Is the register ss, sp, bp, es, fs, gs or ip? */
        bool            is_special;

        /* Can the register be used directly with mov? */
        bool            mov_direct;
    };
};

#define fill(o, is, md)      \
    .opcode = o, .is_special = is, .mov_direct = md

std::vector<std::pair<usint8, struct bit16_opcode_info>> opcode_data{
    std::pair<usint8, struct bit16_opcode_info>(
        0x06, 
        {fill(r_ax_opcode, false, true)}),
    std::pair<usint8, struct bit16_opcode_info>(
        0x07,
        {fill(r_ah_opcode, false, true)}),
    std::pair<usint8, struct bit16_opcode_info>(
        0x08,
        {fill(r_al_opcode, false, true)})
};

#ifdef __cplusplus
}
#endif

#endif