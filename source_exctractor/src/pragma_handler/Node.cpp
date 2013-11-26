

#include "pragma_handler/Node.h"



//using namespace clang;



Node::Node(clang::OMPExecutableDirective *pragma, clang::FunctionDecl *fd, clang::SourceManager& sm){
  
  this->optionVect = new std::map<std::string, varList>();
  this->pragma = pragma;

  if(pragma->getAssociatedStmt()) {
    if(strcmp(pragma->getStmtClassName(), "OMPParallelDirective") == 0 && utils::Line(pragma->getAssociatedStmt()->getLocStart(), sm) == utils::Line(pragma->getAssociatedStmt()->getLocEnd(), sm)){
      setPragmaClauses(sm);
      this->pragma = static_cast<clang::OMPExecutableDirective *>(static_cast<clang::CapturedStmt *>(pragma->getAssociatedStmt())->getCapturedStmt());
      std::cout << this->pragma->getStmtClassName() << std::endl;
    }
  }
  toLocationStruct(sm);
	setParentFunction(fd, sm);	
	setPragmaClauses(sm);

	childrenVect = new std::vector<Node *>();

	if(strcmp(this->pragma->getStmtClassName(), "OMPForDirective") == 0) {
    clang::ForStmt *fs = static_cast<clang::ForStmt *>(static_cast<clang::CapturedStmt *>(this->pragma->getAssociatedStmt())->getCapturedStmt());
    this->forNode = new ForNode(fs);
  } else
		this->forNode = NULL;

}


void Node::toLocationStruct(const clang::SourceManager& sm) {
  
  SourceLocationStruct sL;
  clang::Stmt *s = pragma;
  if(pragma->getAssociatedStmt())
    s = static_cast<clang::CapturedStmt *>(pragma->getAssociatedStmt())->getCapturedStmt();

  if(s != NULL) {
    fileName = utils::FileName(s->getLocStart(), sm);
    startLine = utils::Line(s->getLocStart(), sm);
    startColumn = utils::Column(s->getLocStart(), sm);

    endLine = utils::Line(s->getLocEnd(), sm);
    startColumn = utils::Column(s->getLocEnd(), sm);

  } else {
    fileName = utils::FileName(pragma->getLocStart(), sm);
    startLine = utils::Line(pragma->getLocStart(), sm);
    startColumn = utils::Column(pragma->getLocStart(), sm);

    endLine = utils::Line(pragma->getLocEnd(), sm);
    startColumn = utils::Column(pragma->getLocEnd(), sm);
  }
  return;
}


void Node::setParentFunction(clang::FunctionDecl *functD, const clang::SourceManager& sm) {
  
  fI.fD = functD;
  fI.parentFunctionLine =  utils::Line(functD->getLocStart(), sm);
  fI.parentFunctionLineEnd = utils::Line(functD->getLocEnd(), sm);
/*
 * ---- Name of the function containing the pragma ----
 */
  fI.parentFunctionName = functD->getNameInfo().getAsString();

/*
 * ---- Return type of the function containing the pragma ----
 */
  fI.parentFunctionReturnType = functD->getResultType().getAsString();

/*
 * ---- Parameters of the function containing the pragma ----
 */

  fI.nParams = functD->getNumParams();
  fI.parentFunctionParameter = new std::string*[fI.nParams];
  for(int i = 0; i < fI.nParams; i ++) {
    fI.parentFunctionParameter[i] = new std::string[2];
    
    const clang::ValueDecl *vD = static_cast<const clang::ValueDecl *>(functD->getParamDecl(i));
    fI.parentFunctionParameter[i][0] = vD->getType().getAsString();

    const clang::NamedDecl *nD = static_cast<const clang::NamedDecl *>(functD->getParamDecl(i));            
    fI.parentFunctionParameter[i][1] = nD->getNameAsString();
  }

/*
 * ---- If the parent function is declared in a class return the name of the class ----
 */
/*  if (clang::CXXMethodDecl *cxxMethodD = dynamic_cast<clang::CXXMethodDecl *>(functD)){
    const clang::NamedDecl *nD = static_cast<const clang::NamedDecl *>(cxxMethodD->getParent());            
    fI.parentFunctionClassName = nD->getQualifiedNameAsString();
  }
*/
    fI.parentFunctionClassName = "";
//  }

}




void Node::setPragmaClauses(clang::SourceManager& sm) {

	this->pragmaName = this->pragma->getStmtClassName();
  
/*
 * ---- Exctract pragma options ----
 */
  clang::OMPClause *c = NULL;
  const char * cName;
  unsigned nClauses = pragma->getNumClauses();
  for(unsigned i = 0; i < nClauses; i ++) {
    c = pragma->getClause(i);
    cName = getOpenMPClauseName(c->getClauseKind());
    varList *vl = new varList;
    if(strcmp(cName, "shared") == 0 || strcmp(cName, "private") == 0 || strcmp(cName, "firstprivate") == 0) {
      for(clang::StmtRange range = c->children(); range; ++ range) {
        const clang::DeclRefExpr *dRE = static_cast<const clang::DeclRefExpr *>(*range);
        if(dRE) {
          const clang::NamedDecl *nD = dRE->getFoundDecl();
          const clang::ValueDecl *vD = dRE->getDecl();
          vl->insert(std::pair<std::string, std::string>(nD->getNameAsString(), vD->getType().getAsString()));
        }
      }
    }else if(strcmp(cName, "period") == 0) {
      clang::OMPPeriodClause *pC = static_cast<clang::OMPPeriodClause *>(c);
      const clang::IntegerLiteral *iL = static_cast<const clang::IntegerLiteral *>(pC->getPeriodValue());
      char periodVal[100];
      sprintf(periodVal, "%lu", iL->getValue().getZExtValue());
      vl->insert(std::pair<std::string, std::string>(periodVal, ""));
    }else {
      vl->insert(std::pair<std::string, std::string>("", ""));
    }

    optionVect->insert(std::pair<std::string, varList>(cName, *vl));

  }

}



void Node::createXMLPragma(tinyxml2::XMLDocument *doc, tinyxml2::XMLElement *pragmasElement) {
  tinyxml2::XMLElement *pragmaElement = doc->NewElement("Pragma");
  pragmasElement->InsertEndChild(pragmaElement);

  tinyxml2::XMLElement *nameElement = doc->NewElement("Name");
  pragmaElement->InsertEndChild(nameElement);
  
  tinyxml2::XMLText* nameText = doc->NewText(this->pragmaName.c_str());
  nameElement->InsertEndChild(nameText);

  tinyxml2::XMLElement *positionElement = doc->NewElement("Position");
  
  if(optionVect->size() != 0) {
    tinyxml2::XMLElement *optionsElement = doc->NewElement("Options");
    pragmaElement->InsertEndChild(optionsElement);

    this->createXMLPragmaOptions(doc, optionsElement);

    pragmaElement->InsertAfterChild(optionsElement, positionElement);
  } else {

/*
 * ---- Position ----
 */  
    pragmaElement->InsertEndChild(positionElement);
  }

  tinyxml2::XMLElement *startLineElement = doc->NewElement("StartLine");
  positionElement->InsertEndChild(startLineElement);
  char startLine[100];
  sprintf(startLine, "%d", this->startLine);
  tinyxml2::XMLText* startLineText = doc->NewText(startLine);
  startLineElement->InsertEndChild(startLineText);

  tinyxml2::XMLElement *endLineElement = doc->NewElement("EndLine");
  positionElement->InsertEndChild(endLineElement);
  char endLine[100];
  sprintf(endLine, "%d", this->endLine);
  tinyxml2::XMLText* endtLineText = doc->NewText(endLine);
  endLineElement->InsertEndChild(endtLineText);

/*
 * ----- If present insert info of the For stmt ---- 
 */
  if(this->forNode != NULL) {
    tinyxml2::XMLElement *forElement = doc->NewElement("For");
    pragmaElement->InsertEndChild(forElement);
    this->forNode->createXMLPragmaFor(doc, forElement);
  }

  if(this->childrenVect->size() != 0) {
    tinyxml2::XMLElement *nestingElement = doc->NewElement("Children");
    pragmaElement->InsertEndChild(nestingElement);
    tinyxml2::XMLElement *pragmasElement = doc->NewElement("Pragmas");
    nestingElement->InsertEndChild(pragmasElement);
    for(std::vector<Node *>::iterator itn = this->childrenVect->begin(); itn != this->childrenVect->end(); ++itn) {
      (*itn)->createXMLPragma(doc, pragmasElement);
    }
  }
}



void Node::createXMLPragmaOptions(tinyxml2::XMLDocument *doc, tinyxml2::XMLElement *optionsElement) {
  if(this->optionVect->size() != 0) {
    for(std::map<std::string, varList>::iterator itm =this->optionVect->begin(); itm != this->optionVect->end(); ++ itm) {

      tinyxml2::XMLElement *optionElement = doc->NewElement("Option");
      optionsElement->InsertEndChild(optionElement);

      tinyxml2::XMLElement *optionNameElement = doc->NewElement("Name");
      optionElement->InsertEndChild(optionNameElement);
      tinyxml2::XMLText* nameOptText = doc->NewText((*itm).first.c_str());
      optionNameElement->InsertEndChild(nameOptText);

      if((*itm).second.size() != 0) {
        for(std::map<std::string, std::string>::iterator itv = (*itm).second.begin(); itv != (*itm).second.end(); ++ itv) {
          tinyxml2::XMLElement *parameterElement = doc->NewElement("Parameter");
          optionElement->InsertEndChild(parameterElement);

          if(strcmp((*itv).first.c_str(), "") != 0) {
            tinyxml2::XMLElement *typeElement = doc->NewElement("Type");
            tinyxml2::XMLText* typeText = doc->NewText((*itv).second.c_str());
            typeElement->InsertEndChild(typeText);
            parameterElement->InsertEndChild(typeElement);
          }
          tinyxml2::XMLElement *nameElement = doc->NewElement("Var");
          tinyxml2::XMLText* nameText = doc->NewText((*itv).first.c_str());
          nameElement->InsertEndChild(nameText);
          parameterElement->InsertEndChild(nameElement);

        }
      }
    }
  }

}





/* --------------------------------------------------------------------------------------------------
 * --------------------------------------------------------------------------------------------------
 */

void Node::visitNodeChildren() {

	for(std::vector<Node *>::iterator itn = this->childrenVect->begin(); itn != this->childrenVect->end(); itn ++) {
		std::cout << "Child " << std::endl;
		(*itn)->getPragmaInfo();
   		(*itn)->printNode();

   		std::cout << std::endl;

   		(*itn)->visitNodeChildren();

	}
}

void Node::getPragmaInfo() {

	std::cout << this->pragmaName << std::endl;
  std::cout << "Num clauses = " << this->optionVect->size() << std::endl;
	for(std::map<std::string, varList>:: iterator itm = optionVect->begin(); itm != optionVect->end(); ++itm) {
		std::cout << (*itm).first << ": ";
		for(varList::iterator itv = (*itm).second.begin(); itv != (*itm).second.end(); ++ itv) {      
			std::cout << " " << (*itv).second << " " << (*itv).first;
    }

		std::cout << std::endl;
	}
/*  std::cout << "Associated block type: ";
  if(this->pragma->isStatement())
    std::cout << "stmt  " << this->pragma->getStatement()->getStmtClassName() << std::endl;
  else {
    std::cout << "decl  "; 
    this->pragma->getDecl()->dump();
  }
*/
}

void Node::printNode() {
	std::cout << "Start Line = " << this->startLine << "   End Line = " << this->endLine << std::endl;
	std::cout << "Parent function line = " << this->fI.parentFunctionLine << std::endl;
	
	if(forNode != NULL) {
   		

   		forNode->printFor();
   	}
}
