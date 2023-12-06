#include "common.hpp"
#include "backend/run.hpp"

using namespace AssemblerRun;

int main(int args, char *argv[])
{
	moca_assembler_assert(
		args > 1, 
		EXIT_FAILURE, 
		"Expected assembly file as input.\n")
	
	 asm_run assembler_run(argv[1]);

	return 0;
}