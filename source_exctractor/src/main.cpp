
#include "driver/program.h"
//#include "pragma_handler/create_tree.h"
#include "xml_creator/XMLcreator.h"


int main(int argc, char **argv) { 

	if(argc < 2) {
		llvm::errs() << "Usage: Source_exctractor [<options>] <filename>\n";
     	return 1;
	}

	Program program(argc, argv);

	
	std::vector<Root *> *rootVect = CreateTree(program.getPragmaList(), program.getFunctionList(), program.ccompiler.getSourceManager());


	std::cout << "CIAO uuuu" << std::endl;

	for(std::vector<Root *>::iterator itr = rootVect->begin(); itr != rootVect->end(); ++itr) 
		(*itr)->visitTree();
	
	createXML(rootVect, argv[argc - 1]);

	return 0;
}



/*for(itr = program.getPragmaList()->begin(); itr != program.getPragmaList()->end(); ++ itr) {
		std::cout << (*itr)->getStmtClassName() << " - " << (*itr)->getLocStart().printToString(program.ccompiler.getSourceManager())  << std::endl;
    	clang::Stmt *s = static_cast<clang::CapturedStmt *>((*itr)->getAssociatedStmt())->getCapturedStmt();
    	
      	if(s != NULL){
       		if(strcmp(s->getStmtClassName(), "OMPForDirective") != 0) {
          		std::cout << s->getStmtClassName() << " - " << s->getLocStart().printToString(program.ccompiler.getSourceManager()) << " to " << s->getLocEnd().printToString(program.ccompiler.getSourceManager()) << std::endl;
    	  	}
      	}
	}
	*/