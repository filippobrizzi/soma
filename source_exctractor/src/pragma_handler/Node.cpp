

#include "pragma_handler/Node.h"



Node::Node(clang::OMPExecutableDirective *pragma_stmt, clang::FunctionDecl *funct_decl, clang::SourceManager& sm){
  
  option_vect_ = new std::map<std::string, VarList_>();
  pragma_stmt_ = pragma_stmt;

  if(pragma_stmt->getAssociatedStmt()) {
    if(strcmp(pragma_stmt->getStmtClassName(), "OMPParallelDirective") == 0 && utils::Line(pragma_stmt->getAssociatedStmt()->getLocStart(), sm) == utils::Line(pragma_stmt->getAssociatedStmt()->getLocEnd(), sm)){
      setPragmaClauses(sm);
      pragma_stmt_ = static_cast<clang::OMPExecutableDirective *>(static_cast<clang::CapturedStmt *>(pragma_stmt->getAssociatedStmt())->getCapturedStmt());
    }
  }
  setSourceLocation(sm);
	setParentFunction(funct_decl, sm);	
	setPragmaClauses(sm);

	children_vect_ = new std::vector<Node *>();

	if(strcmp(pragma_stmt_->getStmtClassName(), "OMPForDirective") == 0) {
    clang::ForStmt *for_stmt = static_cast<clang::ForStmt *>(static_cast<clang::CapturedStmt *>(pragma_stmt_->getAssociatedStmt())->getCapturedStmt());
    for_node_ = new ForNode(for_stmt);
  } else
		for_node_ = NULL;

}


void Node::setSourceLocation(const clang::SourceManager& sm) {
  
  clang::Stmt *s = pragma_stmt_;
  if(pragma_stmt_->getAssociatedStmt())
    s = static_cast<clang::CapturedStmt *>(pragma_stmt_->getAssociatedStmt())->getCapturedStmt();

  file_name_ = utils::FileName(pragma_stmt_->getLocStart(), sm);
  if(s != NULL) {
    start_line_ = utils::Line(s->getLocStart(), sm);
    start_column_ = utils::Column(s->getLocStart(), sm);

    end_line_ = utils::Line(s->getLocEnd(), sm);
    end_column_ = utils::Column(s->getLocEnd(), sm);

  } else {
    start_line_ = utils::Line(pragma_stmt_->getLocStart(), sm);
    start_column_ = utils::Column(pragma_stmt_->getLocStart(), sm);

    end_line_ = utils::Line(pragma_stmt_->getLocEnd(), sm);
    end_column_ = utils::Column(pragma_stmt_->getLocEnd(), sm);
  }
  return;
}


void Node::setParentFunction(clang::FunctionDecl *funct_decl, const clang::SourceManager& sm) {
  
  parent_funct_info_.function_decl_ = funct_decl;
  parent_funct_info_.function_start_line_ =  utils::Line(funct_decl->getLocStart(), sm);
  parent_funct_info_.function_end_line_ = utils::Line(funct_decl->getLocEnd(), sm);
  
  /* Name of the function containing the pragma */
  parent_funct_info_.function_name_ = funct_decl->getNameInfo().getAsString();

  /* Return type of the function containing the pragma */
  parent_funct_info_.function_return_type_ = funct_decl->getResultType().getAsString();

  /* Parameters of the function containing the pragma */
  parent_funct_info_.num_params_ = funct_decl->getNumParams();
  parent_funct_info_.function_parameters_ = new std::string*[parent_funct_info_.num_params_];
  
  for(int i = 0; i < parent_funct_info_.num_params_; i ++) {
    parent_funct_info_.function_parameters_[i] = new std::string[2];
    
    const clang::ValueDecl *value_decl = static_cast<const clang::ValueDecl *>(funct_decl->getParamDecl(i));
    parent_funct_info_.function_parameters_[i][0] = value_decl->getType().getAsString();

    const clang::NamedDecl *named_decl = static_cast<const clang::NamedDecl *>(funct_decl->getParamDecl(i));            
    parent_funct_info_.function_parameters_[i][1] = named_decl->getNameAsString();
  }

  /* If the parent function is declared in a class return the name of the class */
/*  if (clang::CXXMethodDecl *cxxMethodD = dynamic_cast<clang::CXXMethodDecl *>(funct_decl)){
      const clang::NamedDecl *nD = static_cast<const clang::NamedDecl *>(cxxMethodD->getParent());            
      parent_funct_info_.parentFunctionClassName = nD->getQualifiedNameAsString();
  }
*/
    parent_funct_info_.function_class_name_ = "";

}




void Node::setPragmaClauses(clang::SourceManager& sm) {
  
  pragma_type_ = pragma_stmt_->getStmtClassName();
/*
 * ---- Exctract pragma options ----
 */
  clang::OMPClause *omp_clause = NULL;
  const char * clause_name;
  unsigned num_clauses = pragma_stmt_->getNumClauses();
  
  for(unsigned i = 0; i < num_clauses; i ++) {
    omp_clause = pragma_stmt_->getClause(i);
    clause_name = getOpenMPClauseName(omp_clause->getClauseKind());
    VarList_ *var_list = new VarList_;

    if(strcmp(clause_name, "shared") == 0 || strcmp(clause_name, "private") == 0 || strcmp(clause_name, "firstprivate") == 0) {
      
      for(clang::StmtRange stmt_range = omp_clause->children(); stmt_range; ++ stmt_range) {
        const clang::DeclRefExpr *decl_ref_expr = static_cast<const clang::DeclRefExpr *>(*stmt_range);
        if(decl_ref_expr) {
          const clang::NamedDecl *named_decl = decl_ref_expr->getFoundDecl();
          const clang::ValueDecl *value_decl = decl_ref_expr->getDecl();
          var_list->insert(std::pair<std::string, std::string>(named_decl->getNameAsString(), value_decl->getType().getAsString()));
        }
      }
    }else if(strcmp(clause_name, "period") == 0) {

      clang::OMPPeriodClause *omp_peroid_clause = static_cast<clang::OMPPeriodClause *>(omp_clause);
      const clang::IntegerLiteral *int_literal = static_cast<const clang::IntegerLiteral *>(omp_peroid_clause->getPeriodValue());
      char period_val[100];
      sprintf(period_val, "%lu", int_literal->getValue().getZExtValue());
      var_list->insert(std::pair<std::string, std::string>(period_val, ""));
    }else {
      var_list->insert(std::pair<std::string, std::string>("", ""));
    }

    option_vect_->insert(std::pair<std::string, VarList_>(clause_name, *var_list));

  }

}



void Node::CreateXMLPragmaNode(tinyxml2::XMLDocument *xml_doc, tinyxml2::XMLElement *pragmas_element) {
  
  tinyxml2::XMLElement *pragma_element = xml_doc->NewElement("Pragma");
  pragmas_element->InsertEndChild(pragma_element);

  tinyxml2::XMLElement *name_element = xml_doc->NewElement("Name");
  pragma_element->InsertEndChild(name_element);
  
  tinyxml2::XMLText* name_text = xml_doc->NewText(pragma_type_.c_str());
  name_element->InsertEndChild(name_text);

  tinyxml2::XMLElement *position_element = xml_doc->NewElement("Position");
  
  if(option_vect_->size() != 0) {
    tinyxml2::XMLElement *options_element = xml_doc->NewElement("Options");
    pragma_element->InsertEndChild(options_element);

    CreateXMLPragmaOptions(xml_doc, options_element);

    pragma_element->InsertAfterChild(options_element, position_element);
  } else {

/*
 * ---- Position ----
 */  
    pragma_element->InsertEndChild(position_element);
  }

  tinyxml2::XMLElement *start_line_element = xml_doc->NewElement("StartLine");
  position_element->InsertEndChild(start_line_element);
  char start_line[100];
  sprintf(start_line, "%d", start_line_);
  tinyxml2::XMLText* start_line_text = xml_doc->NewText(start_line);
  start_line_element->InsertEndChild(start_line_text);

  tinyxml2::XMLElement *end_line_element = xml_doc->NewElement("EndLine");
  position_element->InsertEndChild(end_line_element);
  char end_line[100];
  sprintf(end_line, "%d", end_line_);
  tinyxml2::XMLText* end_line_text = xml_doc->NewText(end_line);
  end_line_element->InsertEndChild(end_line_text);

/*
 * ----- If present insert info of the For stmt ---- 
 */
  if(for_node_) {
    tinyxml2::XMLElement *for_element = xml_doc->NewElement("For");
    pragma_element->InsertEndChild(for_element);
    for_node_->CreateXMLPragmaFor(xml_doc, for_element);
  }

  if(children_vect_->size() != 0) {
    tinyxml2::XMLElement *nesting_element = xml_doc->NewElement("Children");
    pragma_element->InsertEndChild(nesting_element);
    tinyxml2::XMLElement *new_pragmas_element = xml_doc->NewElement("Pragmas");
    nesting_element->InsertEndChild(new_pragmas_element);
    for(std::vector<Node *>::iterator node_itr = children_vect_->begin(); node_itr != children_vect_->end(); ++node_itr) {
      (*node_itr)->CreateXMLPragmaNode(xml_doc, new_pragmas_element);
    }
  }
}



void Node::CreateXMLPragmaOptions(tinyxml2::XMLDocument *xml_doc, tinyxml2::XMLElement *options_element) {
  if(option_vect_->size() != 0) {
    
    for(std::map<std::string, VarList_>::iterator options_itr = option_vect_->begin(); options_itr != option_vect_->end(); ++ options_itr) {

      tinyxml2::XMLElement *option_element = xml_doc->NewElement("Option");
      options_element->InsertEndChild(option_element);

      tinyxml2::XMLElement *option_name_element = xml_doc->NewElement("Name");
      option_element->InsertEndChild(option_name_element);
      tinyxml2::XMLText* name_opt_text = xml_doc->NewText((*options_itr).first.c_str());
      option_name_element->InsertEndChild(name_opt_text);

      if((*options_itr).second.size() != 0) {
        for(std::map<std::string, std::string>::iterator var_itr = (*options_itr).second.begin(); var_itr != (*options_itr).second.end(); ++ var_itr) {
          tinyxml2::XMLElement *parameter_element = xml_doc->NewElement("Parameter");
          option_element->InsertEndChild(parameter_element);

          if(strcmp((*var_itr).first.c_str(), "") != 0) {
            tinyxml2::XMLElement *type_element = xml_doc->NewElement("Type");
            tinyxml2::XMLText* type_text = xml_doc->NewText((*var_itr).second.c_str());
            type_element->InsertEndChild(type_text);
            parameter_element->InsertEndChild(type_element);
          }
          tinyxml2::XMLElement *name_element = xml_doc->NewElement("Var");
          tinyxml2::XMLText* name_text = xml_doc->NewText((*var_itr).first.c_str());
          name_element->InsertEndChild(name_text);
          parameter_element->InsertEndChild(name_element);

        }
      }
    }
  }

}





/* --------------------------------------------------------------------------------------------------
 * --------------------------------------------------------------------------------------------------
 */

void Node::visitNodeChildren() {

	for(std::vector<Node *>::iterator itn = children_vect_->begin(); itn != children_vect_->end(); itn ++) {
		std::cout << "Child " << std::endl;
		(*itn)->getPragmaInfo();
   		(*itn)->printNode();

   		std::cout << std::endl;

   		(*itn)->visitNodeChildren();

	}
}

void Node::getPragmaInfo() {

	std::cout << pragma_type_ << std::endl;
  std::cout << "Num clauses = " << option_vect_->size() << std::endl;
	for(std::map<std::string, VarList_>:: iterator itm = option_vect_->begin(); itm != option_vect_->end(); ++itm) {
		std::cout << (*itm).first << ": ";
		for(VarList_::iterator itv = (*itm).second.begin(); itv != (*itm).second.end(); ++ itv) {      
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
	std::cout << "Start Line = " << start_line_ << "   End Line = " << end_line_ << std::endl;
	std::cout << "Parent function line = " << parent_funct_info_.function_start_line_ << std::endl;
	
	if(for_node_ != NULL) {
   		

   		for_node_->printFor();
   	}
}
