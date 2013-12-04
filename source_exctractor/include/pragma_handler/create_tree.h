
#include "driver/program.h"


std::vector<Root *> *CreateTree(std::vector<clang::OMPExecutableDirective *> *pragma_list,
								std::vector<clang::FunctionDecl *> *function_list, clang::SourceManager &sm);


clang::FunctionDecl *GetFunctionForPragma(clang::OMPExecutableDirective *pragma_stmt, 
										  std::vector<clang::FunctionDecl *> *function_list, 
										  clang::SourceManager &sm);

void BuildTree(Root *root, Node *n);


bool CheckAnnidation(Node *parent, Node *n);

