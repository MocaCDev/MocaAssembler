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
        reg8    = 0x0,
        reg16,
        reg32
    };

    enum class mem_operands
    {
        mem8    = 0x3,
        mem16,
        mem32
    };

    enum class immediate_operands
    {
        imm8    = 0x6,
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

    struct instruction_data
    {
        instructions        instruction_id;

        /* So long as `lval_operand` is `reg8`, `reg16` or `reg32` this will
         * be assigned.
         * */
        RegisterTokens  reg_id;

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
        uslng program_counter = 0;
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

        inline void write_instruction_to_bin()
        {
            switch(idata->reg_id)
            {
                case RegisterTokens::R_ax:
                {
                    /* Instruction opcode. */
                    write_bin(((opcode << 4) | r_ax_opcode));

                    /* Little Endian specific.
                     * Write the value stored in `al`.
                     *
                     * Little Endian means the least significant (rightmost 8 bits) get
                     * written first and the most significant (leftmost 8 bits) get written last.
                     * 
                     * */
                    write_bin(AssemblerCommon::LE_get_rightmost_byte(get_r_ax_value()));

                    /* Write the value stored in `ah`. */
                    write_bin(AssemblerCommon::LE_get_leftmost_byte(get_r_ax_value()));

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