

#include "pragma_handler/ForNode.h"


ForNode::ForNode(clang::ForStmt *fors) {
  this->loopVarType = "";
  this->loopVarInitValSet = false;

  this->loopVarInitVar = "";

  this->conditionValSet = false;
  this->conditionVar = "";

  this->incrementValSet = false;
  this->incrementVar = "";

  forSetParameters(fors);
}


void ForNode::forSetParameters(clang::ForStmt *fs) {

  this->forInitialization(fs);
  this->forCondition(fs);
  this->forIncrement(fs);

}


void ForNode::forInitialization(clang::ForStmt *fs) {
/*
* Initialization of the loop variable
*/

  // for(int i = ....)
  if(strcmp(fs->child_begin()->getStmtClassName(), "DeclStmt") == 0) {
    const clang::DeclStmt *dS = static_cast<const clang::DeclStmt *>(*(fs->child_begin()));
    const clang::Decl *d = dS->getSingleDecl();
/*    
 *  Return the name of the variable
 */
    const clang::NamedDecl *nD = static_cast<const clang::NamedDecl *>(d);            
    this->loopVar = nD->getNameAsString();
    
/*    
 *  Return the type of the variable
 */   
    const clang::ValueDecl *vD = static_cast<const clang::ValueDecl *>(nD);
    this->loopVarType = vD->getType().getAsString();

/*
 *  for (... = 0)
 */
    if(strcmp(dS->child_begin()->getStmtClassName(), "IntegerLiteral") == 0) {      
      const clang::IntegerLiteral *iL = static_cast<const clang::IntegerLiteral *>(*(dS->child_begin())); 
      this->loopVarInitVal = iL->getValue().getZExtValue();
      this->loopVarInitValSet = true;
/*
 *  for (... = a)
 */
    }else if (strcmp(dS->child_begin()->getStmtClassName(), "ImplicitCastExpr") == 0) {
      const clang::DeclRefExpr *dRE = static_cast<const clang::DeclRefExpr *>(*(dS->child_begin()->child_begin()));
      const clang::NamedDecl *nD = dRE->getFoundDecl();
      this->loopVarInitVar = nD->getNameAsString();
    }
  }
/*
 *  for ( i = ...)
 */
  else if(strcmp(fs->child_begin()->getStmtClassName(), "BinaryOperator") == 0) {
    const clang::BinaryOperator *bO = static_cast<const clang::BinaryOperator *>(*(fs->child_begin())); 
    const clang::DeclRefExpr *dRE = static_cast<const clang::DeclRefExpr *>(*(bO->child_begin()));
    
    //Return the name of the variable
    const clang::NamedDecl *nD = dRE->getFoundDecl();
    this->loopVar = nD->getNameAsString();
    
/*
 *  for( ... = 0)
 */
    clang::ConstStmtIterator stI = bO->child_begin();
    stI ++;
    if(strcmp(stI->getStmtClassName(), "IntegerLiteral") == 0) {
      const clang::IntegerLiteral *iL = static_cast<const clang::IntegerLiteral *>(*stI);
      this->loopVarInitVal = iL->getValue().getZExtValue();
      this->loopVarInitValSet = true;      

/*
 *  for ( ... = a)
 */
    } else if (strcmp(stI->getStmtClassName(), "ImplicitCastExpr") == 0) {
      const clang::DeclRefExpr *dRE = static_cast<const clang::DeclRefExpr *>(*(stI->child_begin()));
      const clang::NamedDecl *nD = dRE->getFoundDecl();
      this->loopVarInitVar = nD->getNameAsString();
    }
  }
}


void ForNode::forCondition(clang::ForStmt *fs) {

  const clang::Expr *cE = fs->getCond();
  const clang::BinaryOperator *bO = static_cast<const clang::BinaryOperator *>(cE);
  
/*
 *  Conditional funcion
 */
  this->conditionOp = bO->getOpcodeStr();

/*
 *  Conditional value
 */
  const clang::Expr *rEx = bO->getRHS();
  if(strcmp(rEx->getStmtClassName(), "IntegerLiteral") == 0) {
    const clang::IntegerLiteral *iL = static_cast<const clang::IntegerLiteral *>(rEx);
    this->conditionVal = iL->getValue().getZExtValue();
    this->conditionValSet = true;

  } else if(strcmp(rEx->getStmtClassName(), "ImplicitCastExpr") == 0) {
    const clang::DeclRefExpr *dRE = static_cast<const clang::DeclRefExpr *>(*(rEx->child_begin()));
    const clang::NamedDecl *nD = dRE->getFoundDecl();
    this->conditionVar = nD->getNameAsString(); 
  }
}

void ForNode::forIncrement(clang::ForStmt *fs) {
  
  const clang::Expr *cI = fs->getInc();

  if(strcmp(cI->getStmtClassName(), "UnaryOperator") == 0) {
    const clang::UnaryOperator *uO =  static_cast<const clang::UnaryOperator *>(cI);
    this->incrementOp = uO->getOpcodeStr(uO->getOpcode());

  }else if(strcmp(cI->getStmtClassName(), "CompoundAssignOperator") == 0) {
    const clang::CompoundAssignOperator *cAO = static_cast<const clang::CompoundAssignOperator *>(cI);
    this->incrementOp = cAO->getOpcodeStr();
    const clang::Expr *rEx = cAO->getRHS();

    if(strcmp(rEx->getStmtClassName(), "IntegerLiteral") == 0) {
      const clang::IntegerLiteral *iL = static_cast<const clang::IntegerLiteral *>(rEx);
      this->incrementVal = iL->getValue().getZExtValue();
      this->incrementValSet = true;

    }else if(strcmp(rEx->getStmtClassName(), "ImplicitCastExpr") == 0) {
      const clang::DeclRefExpr *dRE = static_cast<const clang::DeclRefExpr *>(*(rEx->child_begin()));
      const clang::NamedDecl *nD = dRE->getFoundDecl();
      this->incrementVar = nD->getNameAsString();  
    }
  }

}


void ForNode::printFor() {

//Initialization
  std::cout << "for( " << this->loopVarType << " " << this->loopVar << " = ";
  if(this->loopVarInitValSet == true)
     std::cout << this->loopVarInitVal << "; ";
  else
    std::cout << this->loopVarInitVar << "; ";

//Condition
  std::cout << this->loopVar << " " << this->conditionOp << " ";
  if(this->conditionValSet == true)
    std::cout << this->conditionVal << "; ";
  else
    std::cout << this->conditionVar << "; ";

//Increment
  std::cout << this->loopVar << " " << this->incrementOp;
  if(this->incrementValSet == true)
    std::cout << " " << this->incrementVal << ";)" << std::endl;
  else
    std::cout << this->incrementVar << ";)" << std::endl;

}



void ForNode::createXMLPragmaFor(tinyxml2::XMLDocument *doc, tinyxml2::XMLElement *forElement) {

/*
 * ----- DECLARATION -----
 */ 
  tinyxml2::XMLElement *declarationElement = doc->NewElement("Declaration");
  forElement->InsertEndChild(declarationElement);

  tinyxml2::XMLElement *typeElement = doc->NewElement("Type");
  declarationElement->InsertEndChild(typeElement);
  tinyxml2::XMLText* nameTypeText = doc->NewText(this->loopVarType.c_str());
  typeElement->InsertEndChild(nameTypeText);

  tinyxml2::XMLElement *loopVarElement = doc->NewElement("LoopVariable");
  declarationElement->InsertEndChild(loopVarElement);
  tinyxml2::XMLText* nameLoopVarText = doc->NewText(this->loopVar.c_str());
  loopVarElement->InsertEndChild(nameLoopVarText);

  if(this->loopVarInitValSet == true) {
    tinyxml2::XMLElement *initValElement = doc->NewElement("InitValue");
    declarationElement->InsertEndChild(initValElement);
    char loopVarInitVal[100];
    sprintf(loopVarInitVal, "%d", this->loopVarInitVal);
    tinyxml2::XMLText* nameInitValText = doc->NewText(loopVarInitVal);
    initValElement->InsertEndChild(nameInitValText);
  }else {
    tinyxml2::XMLElement *initVarElement = doc->NewElement("InitVariable");
    declarationElement->InsertEndChild(initVarElement);
    tinyxml2::XMLText* nameInitVarText = doc->NewText(this->loopVarInitVar.c_str());
    initVarElement->InsertEndChild(nameInitVarText);
  }

/*
 * ---- CONDITION -----
 */ 
  tinyxml2::XMLElement *conditionElement = doc->NewElement("Condition");
  forElement->InsertAfterChild(declarationElement, conditionElement); 
  
  tinyxml2::XMLElement *conditionOpElement = doc->NewElement("Op");
  conditionElement->InsertEndChild(conditionOpElement);
  tinyxml2::XMLText* conditionOpText = doc->NewText(this->conditionOp.c_str());
  conditionOpElement->InsertEndChild(conditionOpText);

  if(this->conditionValSet == true) {
    tinyxml2::XMLElement *conditionValElement = doc->NewElement("ConditionValue");
    conditionElement->InsertEndChild(conditionValElement);
    char conditionVal[100];
    sprintf(conditionVal, "%d", this->conditionVal);
    tinyxml2::XMLText* conditionValText = doc->NewText(conditionVal);
    conditionValElement->InsertEndChild(conditionValText);
  }else {
    tinyxml2::XMLElement *conditionVarElement = doc->NewElement("ConditionVariable");
    conditionElement->InsertEndChild(conditionVarElement);
    tinyxml2::XMLText* conditionVarText = doc->NewText(this->conditionVar.c_str());
    conditionVarElement->InsertEndChild(conditionVarText);
  }

/*
 * ---- INCREMENT ----
 */
  tinyxml2::XMLElement *incrementElement = doc->NewElement("Increment");
  forElement->InsertAfterChild(conditionElement, incrementElement); 

  tinyxml2::XMLElement *incrementOpElement = doc->NewElement("Op");
  incrementElement->InsertEndChild(incrementOpElement);
  tinyxml2::XMLText* incrementOpText = doc->NewText(this->incrementOp.c_str());
  incrementOpElement->InsertEndChild(incrementOpText);

  if(this->incrementValSet == true) {
    tinyxml2::XMLElement *incrementValElement = doc->NewElement("IncrementValue");
    incrementElement->InsertEndChild(incrementValElement);
    char incrementVal[100];
    sprintf(incrementVal, "%d", this->incrementVal);
    tinyxml2::XMLText* incrementValText = doc->NewText(incrementVal);
    incrementValElement->InsertEndChild(incrementValText);

  }else if(this->incrementVar.compare("") != 0) {
    tinyxml2::XMLElement *incrementVarElement = doc->NewElement("IncrementVariable");
    incrementElement->InsertEndChild(incrementVarElement);
    tinyxml2::XMLText* incrementVarText = doc->NewText(this->incrementVar.c_str());
    incrementVarElement->InsertEndChild(incrementVarText);
  }


}


