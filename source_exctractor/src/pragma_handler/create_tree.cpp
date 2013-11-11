

#include "pragma_handler/create_tree.h"


std::vector<Root *> *CreateTree(std::vector<clang::OMPExecutableDirective *> *pragmaList,
								std::vector<clang::FunctionDecl *> *functionList, clang::SourceManager &sm) {

	clang::FunctionDecl *functionDecl = NULL;
	std::vector<Root *> *rootVect = new std::vector<Root *>();

	std::vector<clang::OMPExecutableDirective *>::iterator itr;
	for(itr = pragmaList->begin(); itr != pragmaList->end(); ++ itr) {

		clang::FunctionDecl *fd = getFunctionForPragma(*itr, functionList, sm);
		Node * n = new Node(*itr, fd, sm);
		
		if(fd != functionDecl) {
			functionDecl = fd;
			Root *root = new Root(n, n->getParentFunctionInfo());
      		n->setParentNode(NULL);
      		root->setLastNode(n);
      		rootVect->insert(rootVect->end(), root);

    	}else {
     		buildTree(rootVect->back(), n);
      		rootVect->back()->setLastNode(n);
    	}  
		
    }

    return rootVect;
}


clang::FunctionDecl *getFunctionForPragma(clang::OMPExecutableDirective *pragma, 
										  std::vector<clang::FunctionDecl *> *functionList, 
										  clang::SourceManager &sm) {
	unsigned startFuncLine, endFuncLine;
	unsigned pragmaLine = utils::Line(pragma->getLocStart(), sm);
	std::vector<clang::FunctionDecl *>::iterator fitr;
	for(fitr = functionList->begin(); fitr != functionList->end(); ++ fitr) {
		startFuncLine = utils::Line((*fitr)->getSourceRange().getBegin(), sm);
		endFuncLine = utils::Line((*fitr)->getSourceRange().getEnd(), sm);
		if(pragmaLine < endFuncLine && pragmaLine > startFuncLine)
			return (*fitr);
	}
	return NULL;
}


/*
 * ---- Attach the node to the correct parent (if the node is node annidated attach it to root) ----
 * THEOREM: A node can be annidated only in its previous node or in the father of the previous node or in the father 
 *          of the father ..... of the previous node. (This is due to the fact that the list of pragmas is ordered based
            on starting line of the associated stmt).
 */ 
void buildTree(Root *root, Node *n) {

  Node *lastNode = root->getLastNode();
  bool annidation;

  while(lastNode != NULL) {
    annidation = checkAnnidation(lastNode, n);

    if(annidation == true) {
      lastNode->addChildNode(n);
      n->setParentNode(lastNode);
      return;

    }else 
      lastNode = lastNode->getParentNode();
  }

  root->addChildNode(n);
  n->setParentNode(NULL);
}

/*
 * ---- Check if n is annidated inside parent: to be annidated it is enough that n->endLine < parent->endLine 
 * (for sure n->startLine < parent->startLine because pragmas are ordered based on their starting line)
 */ 
bool checkAnnidation(Node *parent, Node *n) {

  if(n->getEndLine() < parent->getEndLine())  
    return true;
  else
    return false;
  
}