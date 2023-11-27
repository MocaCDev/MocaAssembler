#ifndef moca_assembler_run_h
#define moca_assembler_run_h
#include "parser.hpp"

using namespace MocaAssembler_Parser;

namespace AssemblerRun
{
    class asm_run : private parser
    {
    public:
        asm_run(cp_int8 filename)
            : parser(filename)
        {}

        ~asm_run() = default;
    };
}

#endif