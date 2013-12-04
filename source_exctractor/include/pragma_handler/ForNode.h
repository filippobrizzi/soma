

#include "xml_creator/tinyxml2.h"

#include "utils/source_locations.h"
#include "clang/AST/ASTConsumer.h"


#include "clang/Sema/Lookup.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/AST/ASTContext.h"
#include "clang/Sema/Scope.h"
#include "clang/Parse/ParseAST.h"

#include <iostream>
#include <string>

#include <stdio.h>
#include <stdlib.h>

class ForNode {

public:
	/* Loop variable */
	std::string loop_var_;
	std::string loop_var_type_;

	/* Loop variable initial value: (number or variable) */
	int loop_var_init_val_;
	bool loop_var_init_val_set_;
	std::string loop_var_init_var_;

	/* Loop condition */
	std::string condition_op_;
	int condition_val_;
	bool condition_val_set_;
	std::string condition_var_;

	/* Loop increment */
	std::string increment_op_;
	int increment_val_;
	bool increment_val_set_;
	std::string increment_var_;

	void ExtractForParameters(clang::ForStmt *for_stmt);

	void ExtractForInitialization(clang::ForStmt *for_stmt);
	void ExtractForCondition(clang::ForStmt *for_stmt);
	void ExtractForIncrement(clang::ForStmt *for_stmt);


	ForNode(clang::ForStmt *for_stmt);
	void CreateXMLPragmaFor(tinyxml2::XMLDocument *xml_doc, tinyxml2::XMLElement *for_element);

	void printFor();

};