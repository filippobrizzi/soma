

#include "pragma_handler/ForNode.h"


ForNode::ForNode(clang::ForStmt *for_stmt) {
  loop_var_type_ = "";
  loop_var_init_val_set_ = false;

  loop_var_init_var_ = "";

  condition_val_set_ = false;
  condition_var_ = "";

  increment_val_set_ = false;
  increment_var_ = "";

  for_stmt_ = for_stmt;
  ExtractForParameters(for_stmt);
}


void ForNode::ExtractForParameters(clang::ForStmt *for_stmt) {
  
  ExtractForInitialization(for_stmt);
  ExtractForCondition(for_stmt);
  ExtractForIncrement(for_stmt);

}


void ForNode::ExtractForInitialization(clang::ForStmt *for_stmt) {
/*
* Initialization of the loop variable
*/

  /* for(int i = ....) */
  if(strcmp(for_stmt->child_begin()->getStmtClassName(), "DeclStmt") == 0) {
    const clang::DeclStmt *decl_stmt = static_cast<const clang::DeclStmt *>(*(for_stmt->child_begin()));
    const clang::Decl *decl = decl_stmt->getSingleDecl();
    
    /* Return the name of the variable */
    const clang::NamedDecl *named_decl = static_cast<const clang::NamedDecl *>(decl);            
    loop_var_ = named_decl->getNameAsString();
    
    /* Return the type of the variable */  
    const clang::ValueDecl *vale_decl = static_cast<const clang::ValueDecl *>(named_decl);
    loop_var_type_ = vale_decl->getType().getAsString();

    /* for (... = 0) */
    if(strcmp(decl_stmt->child_begin()->getStmtClassName(), "IntegerLiteral") == 0) {      
      const clang::IntegerLiteral *int_literal = static_cast<const clang::IntegerLiteral *>(*(decl_stmt->child_begin())); 
      loop_var_init_val_ = int_literal->getValue().getZExtValue();
      loop_var_init_val_set_ = true;
    
    /* for (... = a) */
    }else if (strcmp(decl_stmt->child_begin()->getStmtClassName(), "ImplicitCastExpr") == 0) {
      const clang::DeclRefExpr *decl_ref_expr = static_cast<const clang::DeclRefExpr *>(*(decl_stmt->child_begin()->child_begin()));
      const clang::NamedDecl *named_decl = decl_ref_expr->getFoundDecl();
      loop_var_init_var_ = named_decl->getNameAsString();
    }

  /* for ( i = ...) */
  }else if(strcmp(for_stmt->child_begin()->getStmtClassName(), "BinaryOperator") == 0) {
    const clang::BinaryOperator *binary_op = static_cast<const clang::BinaryOperator *>(*(for_stmt->child_begin())); 
    const clang::DeclRefExpr *decl_ref_expr = static_cast<const clang::DeclRefExpr *>(*(binary_op->child_begin()));
    
    //Return the name of the variable
    const clang::NamedDecl *named_decl = decl_ref_expr->getFoundDecl();
    loop_var_ = named_decl->getNameAsString();
    
    /* for( ... = 0) */
    clang::ConstStmtIterator stmt_itr = binary_op->child_begin();
    stmt_itr ++;
    if(strcmp(stmt_itr->getStmtClassName(), "IntegerLiteral") == 0) {
      const clang::IntegerLiteral *int_literal = static_cast<const clang::IntegerLiteral *>(*stmt_itr);
      loop_var_init_val_ = int_literal->getValue().getZExtValue();      
      loop_var_init_val_set_ = true;      

    /* for ( ... = a) */
    } else if (strcmp(stmt_itr->getStmtClassName(), "ImplicitCastExpr") == 0) {
      const clang::DeclRefExpr *decl_ref_expr = static_cast<const clang::DeclRefExpr *>(*(stmt_itr->child_begin()));
      const clang::NamedDecl *named_decl = decl_ref_expr->getFoundDecl();
      loop_var_init_var_ = named_decl->getNameAsString();
    }
  }
}


void ForNode::ExtractForCondition(clang::ForStmt *for_stmt) {

  const clang::Expr *condition_expr = for_stmt->getCond();
  const clang::BinaryOperator *binary_op = static_cast<const clang::BinaryOperator *>(condition_expr);
  
  /* Conditional funcion */
  condition_op_ = binary_op->getOpcodeStr();

  /* Conditional value */
  const clang::Expr *right_expr = binary_op->getRHS();

  if(strcmp(right_expr->getStmtClassName(), "IntegerLiteral") == 0) {
    const clang::IntegerLiteral *int_literal = static_cast<const clang::IntegerLiteral *>(right_expr);
    condition_val_ = int_literal->getValue().getZExtValue();
    condition_val_set_ = true;

  }else if(strcmp(right_expr->getStmtClassName(), "ImplicitCastExpr") == 0) {
    const clang::DeclRefExpr *decl_ref_expr = static_cast<const clang::DeclRefExpr *>(*(right_expr->child_begin()));
    const clang::NamedDecl *named_decl = decl_ref_expr->getFoundDecl();

/*
 * ---- PROBLEM: If the variable is not defined inside the block (which block?)
 * ----          the NameDecl * is != NULL, but when you try to exctract the name -> segmentation fault!!
 */
    condition_var_ = named_decl->getNameAsString();
  
  }
}




void ForNode::ExtractForIncrement(clang::ForStmt *for_stmt) {
  
  const clang::Expr *increment_expr = for_stmt->getInc();

  if(strcmp(increment_expr->getStmtClassName(), "UnaryOperator") == 0) {
    const clang::UnaryOperator *unary_op =  static_cast<const clang::UnaryOperator *>(increment_expr);
    increment_op_ = unary_op->getOpcodeStr(unary_op->getOpcode());

  }else if(strcmp(increment_expr->getStmtClassName(), "CompoundAssignOperator") == 0) {
    const clang::CompoundAssignOperator *compound_op = static_cast<const clang::CompoundAssignOperator *>(increment_expr);
    increment_op_ = compound_op->getOpcodeStr();
    const clang::Expr *right_expr = compound_op->getRHS();

    if(strcmp(right_expr->getStmtClassName(), "IntegerLiteral") == 0) {
      const clang::IntegerLiteral *int_literal = static_cast<const clang::IntegerLiteral *>(right_expr);
      increment_val_ = int_literal->getValue().getZExtValue();
      increment_val_set_ = true;

    }else if(strcmp(right_expr->getStmtClassName(), "ImplicitCastExpr") == 0) {
      const clang::DeclRefExpr *decl_ref_expr = static_cast<const clang::DeclRefExpr *>(*(right_expr->child_begin()));
      const clang::NamedDecl *named_decl = decl_ref_expr->getFoundDecl();
      increment_var_ = named_decl->getNameAsString();  
    }
  }

}




void ForNode::CreateXMLPragmaFor(tinyxml2::XMLDocument *xml_doc, tinyxml2::XMLElement *for_element) {

/*
 * ----- DECLARATION -----
 */ 
  tinyxml2::XMLElement *declaration_element = xml_doc->NewElement("Declaration");
  for_element->InsertEndChild(declaration_element);

  tinyxml2::XMLElement *type_element = xml_doc->NewElement("Type");
  declaration_element->InsertEndChild(type_element);
  tinyxml2::XMLText* type_text = xml_doc->NewText(loop_var_type_.c_str());
  type_element->InsertEndChild(type_text);

  tinyxml2::XMLElement *loop_var_element = xml_doc->NewElement("LoopVariable");
  declaration_element->InsertEndChild(loop_var_element);
  tinyxml2::XMLText* loop_var_text = xml_doc->NewText(loop_var_.c_str());
  loop_var_element->InsertEndChild(loop_var_text);

  if(loop_var_init_val_set_ == true) {
    tinyxml2::XMLElement *init_val_element = xml_doc->NewElement("InitValue");
    declaration_element->InsertEndChild(init_val_element);
    char loop_var_init_val[100];
    sprintf(loop_var_init_val, "%d", loop_var_init_val_);
    tinyxml2::XMLText* init_val_text = xml_doc->NewText(loop_var_init_val);
    init_val_element->InsertEndChild(init_val_text);
  }else {
    tinyxml2::XMLElement *init_var_element = xml_doc->NewElement("InitVariable");
    declaration_element->InsertEndChild(init_var_element);
    tinyxml2::XMLText* init_var_text = xml_doc->NewText(loop_var_init_var_.c_str());
    init_var_element->InsertEndChild(init_var_text);
  }

/*
 * ---- CONDITION -----
 */ 
  tinyxml2::XMLElement *condition_element = xml_doc->NewElement("Condition");
  for_element->InsertAfterChild(declaration_element, condition_element); 
  
  tinyxml2::XMLElement *condition_op_element = xml_doc->NewElement("Op");
  condition_element->InsertEndChild(condition_op_element);
  tinyxml2::XMLText* condition_op_text = xml_doc->NewText(condition_op_.c_str());
  condition_op_element->InsertEndChild(condition_op_text);

  if(condition_val_set_ == true) {
    tinyxml2::XMLElement *condition_val_element = xml_doc->NewElement("ConditionValue");
    condition_element->InsertEndChild(condition_val_element);
    char condition_val[100];
    sprintf(condition_val, "%d", condition_val_);
    tinyxml2::XMLText* condition_val_text = xml_doc->NewText(condition_val);
    condition_val_element->InsertEndChild(condition_val_text);
  
  }else {
    tinyxml2::XMLElement *condition_var_element = xml_doc->NewElement("ConditionVariable");
    condition_element->InsertEndChild(condition_var_element);
    tinyxml2::XMLText* condition_var_text = xml_doc->NewText(condition_var_.c_str());
    condition_var_element->InsertEndChild(condition_var_text);
  }

/*
 * ---- INCREMENT ----
 */
  tinyxml2::XMLElement *increment_element = xml_doc->NewElement("Increment");
  for_element->InsertAfterChild(condition_element, increment_element); 

  tinyxml2::XMLElement *increment_op_element = xml_doc->NewElement("Op");
  increment_element->InsertEndChild(increment_op_element);
  tinyxml2::XMLText* increment_op_text = xml_doc->NewText(increment_op_.c_str());
  increment_op_element->InsertEndChild(increment_op_text);

  if(increment_val_set_ == true) {
    tinyxml2::XMLElement *increment_val_element = xml_doc->NewElement("IncrementValue");
    increment_element->InsertEndChild(increment_val_element);
    char increment_val[100];
    sprintf(increment_val, "%d", increment_val_);
    tinyxml2::XMLText* increment_val_text = xml_doc->NewText(increment_val);
    increment_val_element->InsertEndChild(increment_val_text);

  }else if(increment_var_.compare("") != 0) {
    tinyxml2::XMLElement *increment_var_element = xml_doc->NewElement("IncrementVariable");
    increment_element->InsertEndChild(increment_var_element);
    tinyxml2::XMLText* increment_var_text = xml_doc->NewText(increment_var_.c_str());
    increment_var_element->InsertEndChild(increment_var_text);
  }


}






void ForNode::printFor() {

//Initialization
  std::cout << "for( " << this->loop_var_type_ << " " << this->loop_var_ << " = ";
  if(this->loop_var_init_val_set_ == true)
     std::cout << this->loop_var_init_val_ << "; ";
  else
    std::cout << this->loop_var_init_var_ << "; ";

//Condition
  std::cout << this->loop_var_ << " " << this->condition_op_ << " ";
  if(this->condition_val_set_ == true)
    std::cout << this->condition_val_ << "; ";
  else
    std::cout << this->condition_var_ << "; ";

//Increment
  std::cout << this->loop_var_ << " " << this->increment_op_;
  if(this->increment_val_set_ == true)
    std::cout << " " << this->increment_val_ << ";)" << std::endl;
  else
    std::cout << this->increment_var_ << ";)" << std::endl;

}

