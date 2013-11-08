
#include "program.h"

int main(int argc, char **argv) { 

	if(argc < 2) {
		llvm::errs() << "Usage: Source_exctractor [<options>] <filename>\n";
     	return 1;
	}

	Program program(argc, argv);



}