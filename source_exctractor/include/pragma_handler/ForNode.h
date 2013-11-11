

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

private:
//Loop variable
	std::string loopVar;
	std::string loopVarType;

//Loop variable initial value: (number or variable)
	int loopVarInitVal;
	bool loopVarInitValSet;
	std::string loopVarInitVar;

//Loop condition
	std::string conditionOp;
	int conditionVal;
	bool conditionValSet;
	std::string conditionVar;

//Loop increment
	std::string incrementOp;
	int incrementVal;
	bool incrementValSet;
	std::string incrementVar;

	void forSetParameters(clang::ForStmt *fs);

	void forInitialization(clang::ForStmt *fs);
	void forCondition(clang::ForStmt *fs);
	void forIncrement(clang::ForStmt *fs);

public:
	ForNode(clang::ForStmt *fors);
	void createXMLPragmaFor(tinyxml2::XMLDocument *doc, tinyxml2::XMLElement *forElement);

	void printFor();

//	~ForNode();

	

};