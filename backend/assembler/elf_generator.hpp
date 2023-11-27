#ifndef moca_assembler_elf_generator_h
#define moca_assembler_elf_generator_h
#include "../../common.hpp"
#include "elfio/elfio.hpp"

using namespace ELFIO;

#if defined __unux__ || __linux__
#define OS_ABI ELFOSABI_LINUX
#define MT EM_386
#elif defined WIN32 || _WIN32
#define OS_ABI ELFOSABI_NONE
#define MT EM_386
#else
#define OS_ABI ELFOSABI_NONE
#define MT EM_386
#endif

namespace MocaAssembler_ElfGenerator
{
    class ElfGenerator
    {
    private:
        elfio writer;
        section *text_section_data;

        void init_writer()
        {
            writer.set_os_abi(OS_ABI);

            /* It's an executable, unless otherwise specified via command line.
             * TODO: Make it to where this will change depending on CLI ordeals.
             * */
            writer.set_type(ET_EXEC);
            writer.set_machine(MT);
        }
    
    protected:
        /* Called only by the assembler, the heir to `ElfGenerator` */
        void init_text_section()
        {
            text_section_data = writer.sections.add(".text");

            /* Per the ELF binary file format spec:
             * The section (`.text`) holds information defined by the program, whose format and meaning are
             * determined solely by the program.
             * */
            text_section_data->set_type(SHT_PROGBITS);

            /* The ELF binary will occupy memory (SHF_ALLOC) and will contain executable machine instructions (per the current section `.text`). */
            text_section_data->set_flags(SHF_ALLOC | SHF_EXECINSTR);

            /* TODO: Does this need to change? */
            text_section_data->set_addr_align(0x10);
        }

    public:
        explicit ElfGenerator()
            : text_section_data(nullptr)
        {
            /* 32-bit ELF binary, little endian. */
            writer.create(ELFCLASS32, ELFDATA2LSB);

            init_writer();
        }

        explicit ElfGenerator(usint8 ELF_CLASS_TYPE, usint8 ENDIAN_TYPE)
            : text_section_data(nullptr)
        {
            writer.create(ELF_CLASS_TYPE, ENDIAN_TYPE);

            init_writer();
        }

        ~ElfGenerator() = default;
    };
}

#endif