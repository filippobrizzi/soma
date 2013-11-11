
#include "pragma_handler/Root.h"


std::vector<Root *> *CreateTree(std::vector<clang::OMPExecutableDirective *> *pragmaList,
								std::vector<clang::FunctionDecl *> *functionList, clang::SourceManager &sm);


clang::FunctionDecl *getFunctionForPragma(clang::OMPExecutableDirective *pragma, 
										  std::vector<clang::FunctionDecl *> *functionList, 
										  clang::SourceManager &sm);

void buildTree(Root *root, Node *n);


bool checkAnnidation(Node *parent, Node *n);

