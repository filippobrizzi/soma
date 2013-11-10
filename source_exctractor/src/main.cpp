
#include "driver/program.h"

#include <iostream>

int main(int argc, char **argv) { 

	if(argc < 2) {
		llvm::errs() << "Usage: Source_exctractor [<options>] <filename>\n";
     	return 1;
	}

	Program program(argc, argv);

	std::vector<clang::OMPExecutableDirective *>::iterator itr;

	for(itr = program.getPragmaList().begin(); itr != program.getPragmaList().end(); ++ itr) {

		std::cout << (*itr)->getStmtClassName() << " - " << (*itr)->getLocStart().printToString(program.ccompiler.getSourceManager())  << std::endl;
    	clang::Stmt *s = static_cast<clang::CapturedStmt *>((*itr)->getAssociatedStmt())->getCapturedStmt();
    	
      	if(s != NULL){
  			std::cout << s->getStmtClassName() << " - " << s->getLocStart().printToString(program.ccompiler.getSourceManager()) << " to " << s->getLocEnd().printToString(program.ccompiler.getSourceManager()) << std::endl;
        	if(strcmp(s->getStmtClassName(), "OMPForDirective") == 0) {
          		clang::Stmt *ss = static_cast<clang::CapturedStmt *>(static_cast<clang::OMPExecutableDirective *>(s)->getAssociatedStmt())->getCapturedStmt();
          		std::cout << ss->getStmtClassName() << " - " << ss->getLocStart().printToString(program.ccompiler.getSourceManager()) << " to " << ss->getLocEnd().printToString(program.ccompiler.getSourceManager()) << std::endl;
        	}
      	}
	}

	return 0;
}