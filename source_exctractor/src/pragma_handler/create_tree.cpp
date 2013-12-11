

#include "pragma_handler/create_tree.h"


std::vector<Root *> *CreateTree(std::vector<clang::OMPExecutableDirective *> *pragma_list,
								std::vector<clang::FunctionDecl *> *function_list, clang::SourceManager &sm) {

	clang::FunctionDecl *function_decl = NULL;
  clang::FunctionDecl *function_decl_tmp = NULL;
	std::vector<Root *> *root_vect = new std::vector<Root *>();
	
  std::vector<clang::OMPExecutableDirective *>::iterator omp_itr;

	for(omp_itr = pragma_list->begin(); omp_itr != pragma_list->end(); ++ omp_itr) {    

    function_decl_tmp = GetFunctionForPragma(*omp_itr, function_list, sm);
    Node * n = new Node(*omp_itr, function_decl_tmp, sm);
      std::cout << "CIAO 1 - " << (*omp_itr)->getStmtClassName() << std::endl;

    /* In case of parallel for skip one stmt. 
       Parallel for is represented with two OMPExecutableDirective,
       (OMPParallel + OMPFor) so we have to skip one stmt */
    if((*omp_itr)->getAssociatedStmt()) {
      if(strcmp((*omp_itr)->getStmtClassName(), "OMPParallelDirective") == 0 
          && utils::Line((*omp_itr)->getAssociatedStmt()->getLocStart(), sm) 
             == utils::Line((*omp_itr)->getAssociatedStmt()->getLocEnd(), sm)) {
        n->pragma_type_ = "OMPParallelForDirective";
        omp_itr++;
      }
    }

    if(function_decl_tmp != function_decl) {
      function_decl = function_decl_tmp;
      Root *root = new Root(n, n->getParentFunctionInfo());
      n->setParentNode(NULL);
      root->setLastNode(n);
      root_vect->push_back(root);

    }else {
      BuildTree(root_vect->back(), n);
      root_vect->back()->setLastNode(n);
    }
  }
  return root_vect;
}


clang::FunctionDecl *GetFunctionForPragma(clang::OMPExecutableDirective *pragma_stmt, 
										  std::vector<clang::FunctionDecl *> *function_list, 
										  clang::SourceManager &sm) {
	
  unsigned funct_start_line, funct_end_line;
	unsigned pragma_start_line = utils::Line(pragma_stmt->getLocStart(), sm);
	std::vector<clang::FunctionDecl *>::iterator funct_itr;

	for(funct_itr = function_list->begin(); funct_itr != function_list->end(); ++ funct_itr) {
		funct_start_line = utils::Line((*funct_itr)->getSourceRange().getBegin(), sm);
		funct_end_line = utils::Line((*funct_itr)->getSourceRange().getEnd(), sm);
		if(pragma_start_line < funct_end_line && pragma_start_line > funct_start_line)
			return (*funct_itr);
	}
	return NULL;
}


/*
 * ---- Attach the node to the correct parent (if the node is node annidated attach it to root) ----
 * THEOREM: A node can be annidated only in its previous node or in the father of the previous node or in the father 
 *          of the father ..... of the previous node. (This is due to the fact that the list of pragmas is ordered based
            on starting line of the associated stmt).
 */ 
void BuildTree(Root *root, Node *n) {

  Node *last_node = root->getLastNode();
  bool annidation;

  while(last_node != NULL) {
    annidation = CheckAnnidation(last_node, n);

    if(annidation == true) {
      last_node->AddChildNode(n);
      n->setParentNode(last_node);
      return;

    }else 
      last_node = last_node->getParentNode();
  }

  root->AddChildNode(n);
  n->setParentNode(NULL);
}

/*
 * ---- Check if n is annidated inside parent: to be annidated it is enough that n->endLine < parent->endLine 
 * (for sure n->startLine < parent->startLine because pragmas are ordered based on their starting line)
 */ 
bool CheckAnnidation(Node *parent, Node *n) {

  if(n->getEndLine() < parent->getEndLine())  
    return true;
  else
    return false;
  
}