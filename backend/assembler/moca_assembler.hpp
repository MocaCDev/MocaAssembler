#ifndef moca_assembler_h
#define moca_assembler_h
#include "asm_common.hpp"
#include "variables.hpp"
#include "elf_generator.hpp"
#include "registers.hpp"
#include "bytecode.hpp"

using namespace MocaAssembler_Variables;
using namespace MocaAssembler_ElfGenerator;
using namespace MocaAssembler_RegisterValues;

namespace MocaAssembler
{
    enum class BitType
    {
        bit16       = 0x0,
        bit32       = 0x1,
        NoneSet     = 0x2
    };

    enum class register_operands
    {
        reg8    = 0x3,
        reg16,
        reg32
    };

    enum class mem_operands
    {
        mem8    = 0x6,
        mem16,
        mem32
    };

    enum class immediate_operands
    {
        imm8    = 0x9,
        imm16,
        imm32
    };

    enum class operand_empty
    {
        Empty = 0xFF
    };

    enum class instructions
    {
        mov
    };

    template<typename LVAL>
        requires (std::is_same<LVAL, register_operands>::value
            || std::is_same<LVAL, mem_operands>::value)
    struct mov_instruction_data
    {
        usint8          lval_type; /* 0x0 = register_operands, 0x1 = mem_operands, 0x2 = immediate_operands. */
        LVAL            lval_id;
        union {
            usint8      register_id;
            usint16     lval_mem_addr;  /* for `mov [addr], reg` */
            usint16     lval_value;     /* for `mov value, reg` */
        } lval;

        /* 0x0 = register_operands (register_id), 
         * 0x1 = mem_operands (lval_mem_addr),
         * 0x2 = immediate_operands (lval_value)
         * */
        usint8          rval_type;
        union {
            usint8      register_id;
            usint16     rval_mem_addr;  /* for `mov reg, [addr]` */
            usint16     rval_value;     /* for `mov reg, value` */
        } rval;

        /* If we are not initializing with an address, it is safe to assume the LVAL is
         * not a mem8/16 operand and, rather, it is a reg8/16.
         * */
        mov_instruction_data()
            : lval_type(0x0) /* register_operands by default. */
        {
            /* TODO: Do we need a `moca_assembler_assert` here? As of right now I do not think we do. */
        }

        /* If we are initializing with an address, it is safe to assume the LVAL is
         * a mem8/16 operand.
         * */
        mov_instruction_data(uslng addr)
            : lval_type(0x1)
        {}

        template<typename T>
            requires std::is_same<T, usint8>::value
                || std::is_same<T, usint16>::value
        void set_LVAL(LVAL id, T lval_value)
        {
            lval_id = id;

            switch(lval_type)
            {
                case 0x0: lval.register_id = lval_value;break;
                case 0x1: lval.lval_mem_addr = lval_value;break;
                case 0x2: lval.lval_value = lval_value;break;
                default: break;
            }
        }

        /* TODO: Figure out if the below template will, at all, be needed. 
        template<typename RVAL>
            requires std::is_same<RVAL, register_operands>::value
                ||  std::is_same<RVAL, mem_operands>::value
                ||  std::is_same<RVAL, immediate_operands>::value*/
        template<typename T>
            requires std::is_same<T, usint8>::value /* register_id */
                || std::is_same<T, usint16>::value  /* lval_mem_addr/lval_value */
        void set_RVAL(usint8 RVAL_id, T value)
        {
            rval_type = RVAL_id;

            switch(rval_type)
            {
                case 0x0: rval.register_id = value;break;
                case 0x1: rval.rval_mem_addr = value;break;
                case 0x2: rval.rval_value = value;break;
                default: moca_assembler_error(UnknownError, "[Unknown Error]\n\tAn unknown error occurred while assigning instruction data.\n")
            }
        }

        ~mov_instruction_data() = default;
    };

    struct instruction_data
    {
        instructions        instruction_id;

        /* So long as `lval_operand` is `reg8`, `reg16` or `reg32` this will
         * be assigned.
         * */
        RegisterTokens      reg_id;

        usint8              lval_operand;
        usint8              rval_operand;
    };

    #define copy_over_instruction_data(a, b)        \
        a->instruction_id = b.instruction_id;        \
        a->reg_id = b.reg_id;                        \
        a->lval_operand = b.lval_operand;            

    class Assembler : private ElfGenerator, protected Variables, protected RegisterValues
    {
    private:
        uslng program_counter = 1;
        uslng origin = 0;
        FILE *bin_data;
        uslng bin_data_index = 0;
        usint8 opcode = 0;
        BitType bit_type;

        /* I really don't need this function, but it does look better
         * code-wise instead of seeing a bunch of `fwrite` function calls.
         * */
        inline void write_bin(usint8 data)
        {
            fwrite(&data, 1, sizeof(usint8), bin_data);
        }

    public:
        struct instruction_data *idata;
        usint8 rval_operand_required[2] = {
            (usint8)operand_empty::Empty,
            (usint8)operand_empty::Empty
        };

        inline void set_rval_operand_to_expect(usint8 first = (usint8)operand_empty::Empty, usint8 second = (usint8)operand_empty::Empty)
        {
           rval_operand_required[0] = first;
           rval_operand_required[1] = second;
        }

        inline void increment_program_counter(uslng amount = 1) { program_counter+=amount; }
        inline void reset_assembler_data()
        {
            program_counter = 1;
            bin_data_index = 0;
            
            if(bin_data) free(bin_data);
        }
        
        inline constexpr uslng get_program_counter() noexcept { return program_counter; }
        inline constexpr uslng get_program_origin() noexcept { return origin; }
        inline constexpr void set_origin(uslng program_origin) noexcept { origin = program_origin; }
        inline constexpr BitType assembler_get_bit_type() noexcept { return bit_type; }
        inline constexpr void assembler_set_bit_type(BitType bt) noexcept { bit_type = bt; }

        inline void assembler_init_new_instruction(instructions instruction)
        {
            if(!idata) idata = new struct instruction_data;

            idata->instruction_id = instruction;

            switch(instruction)
            {
                case instructions::mov: opcode = mov_opcode;break;
                default: break;
            }

            increment_program_counter();
        }

        inline void assembler_set_lval(usint8 r_id)
        {
            /* TODO: Add support for 32-bit. */
            switch(r_id)
            {
                case (usint8)RegisterTokens::R_ax:
                case (usint8)RegisterTokens::R_bx:
                case (usint8)RegisterTokens::R_cx:
                case (usint8)RegisterTokens::R_dx: {
                    idata->rval_operand = (usint8)register_operands::reg16;
                    idata->reg_id = (RegisterTokens)r_id;
                    
                    set_rval_operand_to_expect(
                        (usint8)register_operands::reg16,
                        (usint8)immediate_operands::imm16);
                    break;
                }
                case (usint8)RegisterTokens::R_ah:
                case (usint8)RegisterTokens::R_al:
                case (usint8)RegisterTokens::R_bh:
                case (usint8)RegisterTokens::R_bl:
                case (usint8)RegisterTokens::R_ch:
                case (usint8)RegisterTokens::R_cl:
                case (usint8)RegisterTokens::R_dh:
                case (usint8)RegisterTokens::R_dl: {
                    idata->rval_operand = (usint8)register_operands::reg8;
                    idata->reg_id = (RegisterTokens)r_id;

                    set_rval_operand_to_expect(
                        (usint8)register_operands::reg8,
                        (usint8)immediate_operands::imm8);
                    break;
                }
                default: moca_assembler_error(InvalidRegID, "Invalid register ID recieved by `assembler_set_rval`.\n")
            }

            increment_program_counter();
        }

        inline void assembler_set_rval_imm(struct instruction_data d, uslng value, uslng line, p_int8 filename)
        {
            idata = new instruction_data;

            copy_over_instruction_data(idata, d);
            
            if(rval_operand_required[1] == (usint8)immediate_operands::imm8)
            {
                moca_assembler_assert(
                    value <= max_byte_size,
                    OverflowError,
                    "[Overflow Error]\n\tOn line %ld in `%s`, the RVAL surpasses the max size of a byte (2-byte value) (max size: 0x%X (%d), assignment: 0x%lX (%ld)).\n",
                    line, filename, max_byte_size, max_byte_size, value, value)

                idata->rval_operand = (usint8)immediate_operands::imm8;

                goto assign;
            }

            /* If we hit here, the RVAL is presumably a 16-bit value.
             * Check to make sure the value of the variable does not exceed the
             * max size of a word.
             * */
            moca_assembler_assert(
                value <= max_word_size,
                OverflowError,
                "[Overflow Error]\n\tOn line %ld in `%s`, the RVAL surpasses the max size of a word (2-byte value) (max size: 0x%X (%d), assignment: 0x%lX (%ld)).\n",
                line, filename, max_word_size, max_word_size, value, value)

            idata->rval_operand = (usint8)immediate_operands::imm16;

            assign:
            assign_parent_register_value(
                (usint8)idata->reg_id,
                AssemblerCommon::LE_16(value));
            
            increment_program_counter();
        }

        template<typename T>
        requires (std::is_same<T, register_operands>::value
            || std::is_same<T, mem_operands>::value)
        inline void write_instruction_to_bin(struct mov_instruction_data<T> midata)
        {
            const auto LVAL_REG_get_instruction_opcode = [this, &midata](MovTypes mtype)
            {
                switch(mtype)
                {
                    case MovTypes::regular_mov: return reg_mov_instructions[midata.lval.register_id];
                    default: return (int8)-1;
                }
            };

            /* Generate opcode based on the LVAL being a register. */
            const auto LVAL_REG_generate_opcode = [this, &midata, &LVAL_REG_get_instruction_opcode](bool is_reg8)
            {
                switch(midata.rval_type)
                {
                    case 0x0: /* register_operands */
                    {
                        break;
                    }
                    case 0x1: /* mem_operands */
                    {
                        break;
                    }
                    case 0x2: /* immediate_operands */
                    {
                        usint8 opcode = (usint8)LVAL_REG_get_instruction_opcode(MovTypes::regular_mov);
                        write_bin(opcode);
                        
                        if(is_reg8)
                            write_bin(midata.rval.rval_value & 0xFF);
                        else
                            write_bin(midata.rval.rval_value & 0xFFFF);

                        break;
                    }
                }
            };

            switch(midata.lval_type)
            {
                case 0x0: /* register_operands */
                {
                    switch(midata.lval_id)
                    {
                        case register_operands::reg8:LVAL_REG_generate_opcode(true);break;
                        case register_operands::reg16:LVAL_REG_generate_opcode(false);break;
                        default: break;
                    }
                    break;
                }
                case 0x1: /* mem_operands */
                {
                    break;
                }
                default: break;
            }
        }

        /* Same concept as `write_bin`. This is not needed,
         * but it's better to read.
         *
         * Also, it is probably needed due to `bin_data` being protected
         * in this context.
         * */
        inline void write_to_binary()
        {
            fclose(bin_data);
        }

        template<typename T>
            requires std::is_same<T, usint8>::value
                || std::is_same<T, usint16>::value
                || std::is_same<T, usint32>::value
        void assembler_add_to_bin_data(T data)
        {
            switch(sizeof(T))
            {
                case 1: {
                    break;
                }
                case 2: {
                    break;
                }
                case 4: {
                    break;
                } 
                default: break;
            }
        }

        explicit Assembler()
            : ElfGenerator(), Variables(), bin_data(nullptr),
              bit_type(BitType::NoneSet), idata(nullptr),
              RegisterValues()
        {
            bin_data = fopen("test.bin", "wb");

            moca_assembler_assert(
                bin_data,
                AllocationError,
                "[Allocation Error]\n\tThere was an error allocating memory for binary data.\n"
            )
        }

        explicit Assembler(usint8 ELF_CLASS_TYPE, usint8 ENDIAN_TYPE)
            : ElfGenerator(ELF_CLASS_TYPE, ENDIAN_TYPE), Variables(),
              bit_type(BitType::NoneSet), idata(nullptr)
        {
            bin_data = fopen("test.bin", "wb");

            moca_assembler_assert(
                bin_data,
                AllocationError,
                "[Allocation Error]\n\tThere was an error allocating memory for binary data.\n"
            )
        }

        Assembler& return_assembler()
        {
            this->program_counter = 0;
            this->bin_data_index = 0;

            return *this;
        }

        ~Assembler()
        {
            if(bin_data) free(bin_data);
            if(idata) delete idata;
        }
    };
}

#endif