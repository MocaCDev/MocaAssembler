#ifndef moca_assembler_h
#define moca_assembler_h
#include "../../common.hpp"
#include "variables.hpp"
#include "elf_generator.hpp"

using namespace MocaAssembler_Variables;
using namespace MocaAssembler_ElfGenerator;

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

    enum class instructions
    {
        mov
    };

    struct instruction_data
    {
        instructions        instruction_id;

        usint8              lval_operand;
        usint8              rval_operand;
    };

    class Assembler : private ElfGenerator, protected Variables
    {
    protected:
        uslng program_counter = 0;
        uslng origin = 0;
        p_usint8 bin_data;
        uslng bin_data_index = 0;
        BitType bit_type;
        struct instruction_data *idata;

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
        }

        void assembler_add_lval()
        {}

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

    public:
        explicit Assembler()
            : ElfGenerator(), Variables(), bin_data(nullptr),
              bit_type(BitType::NoneSet), idata(nullptr)
        {
            bin_data = reinterpret_cast<p_usint8>(calloc(1, sizeof(*bin_data)));

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
            bin_data = reinterpret_cast<p_usint8>(calloc(1, sizeof(*bin_data)));

            moca_assembler_assert(
                bin_data,
                AllocationError,
                "[Allocation Error]\n\tThere was an error allocating memory for binary data.\n"
            )
        }

        ~Assembler()
        {
            if(bin_data) free(bin_data);
            if(idata) delete idata;
        }
    };
}

#endif