
//#include "driver/program.h"
#include "xml_creator/XMLcreator.h"


int main(int argc, char **argv) { 

	if(argc < 2) {
		llvm::errs() << "Usage: Source_exctractor [<options>] <filename>\n";
     	return 1;
	}
/*
 * ---- Create a clang::compiler object and launch the parser saving the pragma stmt. 
 * 		Rewrite the sourcecode adding profiling call.
 */
	Program program(argc, argv);

/*
 * ---- With the information exctracted by the parser create a linked list tree of objects containing
 *		all the necessary information of the pragmas.
 */
	std::vector<Root *> *root_vect = CreateTree(program.getPragmaList(), program.getFunctionList(), program.ccompiler_.getSourceManager());

/*
 * ---- Using the tree above create an xml file containing the pragma info. This file is used to produce the scheduler.
 */
	CreateXML(root_vect, argv[argc - 1]);

	for(std::vector<Root *>::iterator itr = root_vect->begin(); itr != root_vect->end(); ++itr) 
		(*itr)->VisitTree();

/*
 * ---- Parse the sourcecode and rewrite it substituting pragmas with function calls. This new file
 * 		will be used with the scheduler to produce the final output.
 */
	Program program2(argc, argv, root_vect);
	
	return 0;
}

