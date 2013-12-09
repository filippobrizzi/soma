
#include "driver/program.h"


void Program::ParseSourceCode(std::string file_name) {

  /* Convert <file>.c to <file_profile>.c */
  std::string out_filename_profile (file_name);
  size_t ext = out_filename_profile.rfind(".");
  if (ext == std::string::npos)
  	ext = out_filename_profile.length();
  out_filename_profile.insert(ext, "_profile");

  llvm::errs() << "Output to: " << out_filename_profile << "\n";
  std::string out_error_info;
  llvm::raw_fd_ostream out_file_profile(out_filename_profile.c_str(), out_error_info, 0);

  /* Create the rewriter object to create the profiling file */
  clang::Rewriter rewrite_profiling;
  rewrite_profiling.setSourceMgr(ccompiler_.getSourceManager(), ccompiler_.getLangOpts());
  
  ProfilingASTConsumer ast_consumer(rewrite_profiling, ccompiler_.getSourceManager());
  /* Parse the AST with the custom ASTConsumer */
  clang::ParseAST(ccompiler_.getPreprocessor(), &ast_consumer, ccompiler_.getASTContext());
	ccompiler_.getDiagnosticClient().EndSourceFile();

  /* Save the pragma and function list */
	pragma_list_ = new std::vector<clang::OMPExecutableDirective *>(ast_consumer.recursive_visitor_.pragma_list_);
	function_list_ = new std::vector<clang::FunctionDecl *>(ast_consumer.recursive_visitor_.function_list_);

  /*Output rewritten source code into a new file */
	const clang::RewriteBuffer *rewrite_buf_profiling = 
      rewrite_profiling.getRewriteBufferFor(ccompiler_.getSourceManager().getMainFileID());  	
  
  out_file_profile << std::string(rewrite_buf_profiling->begin(), rewrite_buf_profiling->end());
  out_file_profile.close();

}


/*
 * ---- Insert the call to the profilefunction tracker to track the execution time of each funcion.
 */
bool ProfilingRecursiveASTVisitor::VisitFunctionDecl(clang::FunctionDecl *f) {     
    
  clang::SourceLocation start_src_loc = f->getLocStart();

  /* Skip function belonging to external include file and not defined function */
  if(sm.getFileID(start_src_loc) == sm.getMainFileID() && f->hasBody() == true) {
  	
    function_list_.push_back(f);

    /* Include the path to ProfileTracker.h */
    if(include_inserted_ == false) {
      std::string text_include = 
      "#include \"/home/pippo/Documents/Library/clomp-master/include/myprogram/profiling/ProfileTracker.h\"\n";
      
      rewrite_profiling_.InsertText(start_src_loc, text_include, true, false);
      include_inserted_ = true;
    } 

    start_src_loc = f->getBody()->getLocStart();
  	unsigned start_line = utils::Line(start_src_loc, sm);
    clang::SourceLocation new_start_src_loc = sm.translateLineCol(sm.getMainFileID(), start_line + 1, 1);
    std::stringstream text_profiling;
    text_profiling << "if( ProfileTracker x = ProfileTrackParams(" << start_line << ", 0)) {\n";
    
    /* Insert the if in the first line of the function definition */
    rewrite_profiling_.InsertText(new_start_src_loc, text_profiling.str(), true, false);

    clang::SourceLocation end_src_loc = f->getLocEnd();
    std::stringstream text_end_bracket;
    text_end_bracket << "}\n";
    /* Close the if bracket at the end of the function */
    rewrite_profiling_.InsertText(end_src_loc, text_end_bracket.str(), true, false);
  }

  return true; 
}



bool ProfilingRecursiveASTVisitor::VisitStmt(clang::Stmt *s) {

	clang::SourceLocation start_src_loc = s->getLocStart();
	if(sm.getFileID(start_src_loc) == sm.getMainFileID()) {
      /* We want just the OpenMP stmt and no duplicate */
  		if (clang::isa<clang::OMPExecutableDirective>(s) && s != previous_stmt_) {
  			previous_stmt_ = s;
  			clang::OMPExecutableDirective *omp_stmt = static_cast<clang::OMPExecutableDirective *>(s);
  			pragma_list_.push_back(omp_stmt);
  			
        clang::Stmt *associated_stmt = omp_stmt->getAssociatedStmt();
        if(associated_stmt) {
          clang::Stmt *captured_stmt = static_cast<clang::CapturedStmt *>(associated_stmt)->getCapturedStmt();
          /* In the case of #omp parallel for we have to go down two level befor finding the ForStmt */
  		    if(strcmp(captured_stmt->getStmtClassName(), "OMPForDirective") != 0)
            RewriteProfiling(captured_stmt);
        }
      }
  	}
  	
  return true;
}


void ProfilingRecursiveASTVisitor::RewriteProfiling(clang::Stmt *s) {
  	
  	clang::SourceLocation start_src_loc = s->getLocStart();
  	unsigned pragma_start_line = utils::Line(start_src_loc,sm);
  	unsigned function_start_line = GetFunctionLineForPragma(s->getLocStart());

    std::stringstream text_profiling;
    if(clang::isa<clang::ForStmt>(s)) {
      std::string condition_var_value = ForConditionVarValue(s);
      //std::string conditionVar = "";
      text_profiling << "if( ProfileTracker x = ProfileTrackParams(" 
          << function_start_line << ", " << pragma_start_line << ", " << condition_var_value << "))\n";
      rewrite_profiling_.InsertText(start_src_loc, text_profiling.str(), true, true);

    } else {
	    text_profiling << "if( ProfileTracker x = ProfileTrackParams(" 
          << function_start_line << ", " << pragma_start_line << "))\n";
      rewrite_profiling_.InsertText(start_src_loc, text_profiling.str(), true, true);
    }

    /* Comment the pragma in the profiling file */
    clang::SourceLocation pragma_start_src_loc = 
        sm.translateLineCol(sm.getMainFileID(), pragma_start_line - 1, 1);
    
    rewrite_profiling_.InsertText(pragma_start_src_loc, "//", true, false);
}




std::string ProfilingRecursiveASTVisitor::ForConditionVarValue(const clang::Stmt *s) {
	
  const clang::ForStmt *for_stmt = static_cast<const clang::ForStmt *>(s);
  const clang::Expr *condition_expr = for_stmt->getCond();
  const clang::BinaryOperator *binary_op = static_cast<const clang::BinaryOperator *>(condition_expr);
  
  std::string start_cond_var_value, end_cond_var_value;

/*
 *  Condition end value
 */
  const clang::Expr *right_expr = binary_op->getRHS();

  if(strcmp(right_expr->getStmtClassName(), "IntegerLiteral") == 0) {
    const clang::IntegerLiteral *int_literal = static_cast<const clang::IntegerLiteral *>(right_expr);
    std::stringstream text_end_value;
    text_end_value << int_literal->getValue().getZExtValue();
    //return text.str();
    end_cond_var_value = text_end_value.str();

  } else if(strcmp(right_expr->getStmtClassName(), "ImplicitCastExpr") == 0) {
  	const clang::DeclRefExpr *decl_ref_expr = 
        static_cast<const clang::DeclRefExpr *>(*(right_expr->child_begin()));
  	
    const clang::NamedDecl *named_decl = decl_ref_expr->getFoundDecl();
  	//return nD->getNameAsString(); 
    end_cond_var_value = named_decl->getNameAsString();
  }

/*
 * Condition start value
 */

/*
 *  for (int i = ...)
 */
  if(strcmp(for_stmt->child_begin()->getStmtClassName(), "DeclStmt") == 0) {
    const clang::DeclStmt *decl_stmt = static_cast<const clang::DeclStmt *>(*(for_stmt->child_begin()));
    const clang::Decl *decl = decl_stmt->getSingleDecl();

/*
 *  for (... = 0)
 */
    if(strcmp(decl_stmt->child_begin()->getStmtClassName(), "IntegerLiteral") == 0) {      
      const clang::IntegerLiteral *int_literal = 
          static_cast<const clang::IntegerLiteral *>(*(decl_stmt->child_begin())); 
      
      std::stringstream text_star_value;
      text_star_value << int_literal->getValue().getZExtValue();
      start_cond_var_value = text_star_value.str();

/*
 *  for (... = a)
 */
    }else if (strcmp(decl_stmt->child_begin()->getStmtClassName(), "ImplicitCastExpr") == 0) {
      const clang::DeclRefExpr *decl_ref_expr = 
          static_cast<const clang::DeclRefExpr *>(*(decl_stmt->child_begin()->child_begin()));
      
      const clang::NamedDecl *named_decl = decl_ref_expr->getFoundDecl();
      start_cond_var_value = named_decl->getNameAsString();
    }
  }
/*
 *  for ( i = ...)
 */
  else if(strcmp(for_stmt->child_begin()->getStmtClassName(), "BinaryOperator") == 0) {
    const clang::BinaryOperator *binary_op = 
        static_cast<const clang::BinaryOperator *>(*(for_stmt->child_begin())); 
    const clang::DeclRefExpr *decl_ref_expr = 
        static_cast<const clang::DeclRefExpr *>(*(binary_op->child_begin()));
/*
 *  for( ... = 0)
 */
    clang::ConstStmtIterator stmt_itr = binary_op->child_begin();
    stmt_itr ++;
    if(strcmp(stmt_itr->getStmtClassName(), "IntegerLiteral") == 0) {
      const clang::IntegerLiteral *int_literal = static_cast<const clang::IntegerLiteral *>(*stmt_itr);
      start_cond_var_value = int_literal->getValue().getZExtValue();      
/*
 *  for ( ... = a)
 */
    } else if (strcmp(stmt_itr->getStmtClassName(), "ImplicitCastExpr") == 0) {
      const clang::DeclRefExpr *decl_ref_expr = 
          static_cast<const clang::DeclRefExpr *>(*(stmt_itr->child_begin()));
      const clang::NamedDecl *named_decl = decl_ref_expr->getFoundDecl();
      start_cond_var_value = named_decl->getNameAsString();
    }
  }
  end_cond_var_value.append(" - ");
  end_cond_var_value.append(start_cond_var_value);
  return end_cond_var_value;
}

unsigned ProfilingRecursiveASTVisitor::GetFunctionLineForPragma(clang::SourceLocation sl) {

	unsigned pragma_line = utils::Line(sl, sm);
	
  unsigned start_func_line, end_func_line;
	std::vector<clang::FunctionDecl *>::iterator func_itr;

	for(func_itr = function_list_.begin(); func_itr != function_list_.end(); ++ func_itr) {
		start_func_line = utils::Line((*func_itr)->getSourceRange().getBegin(), sm);
		end_func_line = utils::Line((*func_itr)->getSourceRange().getEnd(), sm);
		if(pragma_line < end_func_line && pragma_line > start_func_line)
			return start_func_line;
	}

	return 0;
}
/*
 * -------------------------------------------------------------------------------------------------------------------
 * -------------------------------------------------------------------------------------------------------------------
 */

void Program::ParseSourceCode(std::string fileName, std::vector<Root *> *root_vect) {

  /* Convert <file>.c to <file_transformed>.c */
  std::string out_name_pragma (fileName);
  size_t ext = out_name_pragma.rfind(".");
  if (ext == std::string::npos)
    ext = out_name_pragma.length();
  out_name_pragma.insert(ext, "_tranformed");

  llvm::errs() << "Output to: " << out_name_pragma << "\n";
  std::string out_error_info;
  llvm::raw_fd_ostream out_file_pragma(out_name_pragma.c_str(), out_error_info, 0);  

  clang::Rewriter rewrite_pragma;
  rewrite_pragma.setSourceMgr(ccompiler_.getSourceManager(), ccompiler_.getLangOpts());

  TransformASTConsumer t_ast_consumer(rewrite_pragma, root_vect, ccompiler_.getSourceManager());
  
  /* Parse the AST */
  clang::ParseAST(ccompiler_.getPreprocessor(), &t_ast_consumer, ccompiler_.getASTContext());
  ccompiler_.getDiagnosticClient().EndSourceFile();

  const clang::RewriteBuffer *rewrite_buff_pragma = 
      rewrite_pragma.getRewriteBufferFor(ccompiler_.getSourceManager().getMainFileID());
  out_file_pragma << std::string(rewrite_buff_pragma->begin(), rewrite_buff_pragma->end());
  out_file_pragma.close();
}


bool TransformRecursiveASTVisitor::VisitFunctionDecl(clang::FunctionDecl *f) {     
  clang::SourceLocation f_start_src_loc = f->getLocStart();

  if(sm.getFileID(f_start_src_loc) == sm.getMainFileID() && !clang::isa<clang::CXXMethodDecl>(f)) {
    if(include_inserted_ == false) {
      include_inserted_ = true;

      std::string text_include = "#include \"/home/pippo/Documents/Project/soma/source_exctractor/src/thread_pool/threads_pool.h\"\n";

      rewrite_pragma_.InsertText(f_start_src_loc, text_include, true, false);
    }
  }

  return true;
}

bool TransformRecursiveASTVisitor::VisitStmt(clang::Stmt *s) {
  
  clang::SourceLocation s_start_stc_loc = s->getLocStart();
  /* Visit only stmt in the source file (not in included file) and that are pragma stmt */
  if(sm.getFileID(s_start_stc_loc) == sm.getMainFileID() 
        && clang::isa<clang::OMPExecutableDirective>(s) 
        && s != previous_stmt_) {
    
    previous_stmt_ = s;
    clang::OMPExecutableDirective *omp_stmt = static_cast<clang::OMPExecutableDirective *>(s);
    clang::Stmt *associated_stmt = omp_stmt->getAssociatedStmt();
    if(associated_stmt) {
      clang::Stmt *captured_stmt = static_cast<clang::CapturedStmt *>(associated_stmt)->getCapturedStmt();
      if(strcmp(captured_stmt->getStmtClassName(), "OMPForDirective") != 0)
        RewriteOMPPragma(associated_stmt);

    }else if(strcmp(omp_stmt->getStmtClassName(), "OMPBarrierDirective") == 0){
        RewriteOMPBarrier(omp_stmt);
    }
  }
  return true;
}


void TransformRecursiveASTVisitor::RewriteOMPBarrier(clang::OMPExecutableDirective *omp_stmt) {
  unsigned stmt_start_line = utils::Line(omp_stmt->getLocStart(), sm);
  
  std::stringstream text_barrier;
  text_barrier <<
"{\n\
  class Nested : public NestedBase {\n\
  public: \n\
    Nested(int pragma_id) : NestedBase(pragma_id) {}\n\
    void callme(ForParameter for_param){}\n\
  };\n\
  ThreadPool::getInstance(\"" << utils::FileName(omp_stmt->getLocStart(), sm) 
      << "\")->call(std::make_shared<Nested>(" << stmt_start_line << "));\n\
}";

  clang::SourceLocation pragma_start_src_loc = sm.translateLineCol(sm.getMainFileID(), stmt_start_line + 1, 1);
  rewrite_pragma_.InsertText(pragma_start_src_loc, text_barrier.str(), true, false);

  pragma_start_src_loc = sm.translateLineCol(sm.getMainFileID(), stmt_start_line, 1);
  rewrite_pragma_.InsertText(pragma_start_src_loc, "//", true, false);
}


void TransformRecursiveASTVisitor::RewriteOMPPragma(clang::Stmt *associated_stmt) {
  
  clang::Stmt *s = static_cast<clang::CapturedStmt *>(associated_stmt)->getCapturedStmt();

  clang::SourceLocation stmt_start_src_loc = s->getLocStart();
  unsigned pragma_start_line = utils::Line(stmt_start_src_loc, sm);

  Node *n = GetNodeObjForPragma(s);
  
  std::stringstream text;
  std::stringstream text_constructor_params;
  std::stringstream text_class_var;
  std::stringstream text_fx_var;
  std::stringstream text_constructor_var;
  std::stringstream text_constructor;

/* Insert before pragma */
  text <<
"{\n\
  class Nested : public NestedBase {\n\
  public: \n\
    Nested(int pragma_id";

  text_constructor << " : NestedBase(pragma_id)";

  clang::CapturedStmt *captured_stmt = static_cast<clang::CapturedStmt *>(associated_stmt);
  /* Iterate over all the variable used inside a pragma but defined outside. These variable have to be passed to
     the newly created function */ 
  for(clang::CapturedStmt::capture_iterator capture_var_itr = captured_stmt->capture_begin(); 
      capture_var_itr != captured_stmt->capture_end(); 
      ++capture_var_itr){

    clang::VarDecl *var_decl = capture_var_itr->getCapturedVar(); 
    std::string var_type = var_decl->getType().getAsString();

    if(capture_var_itr != captured_stmt->capture_begin()){
      text_fx_var << ", ";
      text_constructor_var << ", ";
      text_constructor_params << ", ";
    }else
      text << ", ";

    if(var_type.find("class") != std::string::npos)
      var_type.erase(0, 6);

    if(n->option_vect_->find("private") != n->option_vect_->end()) {
      if(n->option_vect_->find("private")->second.find(var_decl->getNameAsString()) 
            != n->option_vect_->find("private")->second.end() 
            || var_type.find("*") != std::string::npos){

        text_constructor_params << var_type << " " << var_decl->getNameAsString();
        text_class_var << var_type << " " << var_decl->getNameAsString() << "_;\n";

      }else{
        text_constructor_params << var_type << " & " << var_decl->getNameAsString();
        text_class_var << var_type << " & " << var_decl->getNameAsString() << "_;\n";
      }
    }else if(var_type.find("*") != std::string::npos) {
      text_constructor_params << var_type << " " << var_decl->getNameAsString();
      text_class_var << var_type << " " << var_decl->getNameAsString() << "_;\n";

    }else {
      text_constructor_params << var_type << " & " << var_decl->getNameAsString();
      text_class_var << var_type << " & " << var_decl->getNameAsString() << "_;\n";
    }

    text_constructor << ", " << var_decl->getNameAsString() << "_(" << var_decl->getNameAsString() << ") ";
    text_fx_var << var_decl->getNameAsString() << "_";
    text_constructor_var << var_decl->getNameAsString();
  }

  text << text_constructor_params.str() << ") " << text_constructor.str() 
      << "{}\n" << text_class_var.str() << "\n";    
  
  unsigned stmt_start_line = utils::Line(s->getLocStart(), sm);
  
  if(text_constructor_params.str().compare("") == 0)
      text << "void fx(ForParameter for_param)";
    else
      text << "void fx(ForParameter for_param, " << text_constructor_params.str() <<")";
  
  if(n->for_node_ != NULL) {
    
    std::string text_for;
    text_for = RewriteOMPFor(n);

    text << " {\n" << text_for;
    clang::SourceLocation for_src_loc = sm.translateLineCol(sm.getMainFileID(), stmt_start_line + 1, 1);
    rewrite_pragma_.InsertText(for_src_loc, text.str(), true, false);
    rewrite_pragma_.InsertText(stmt_start_src_loc, "//", true, false);
    
    unsigned stmt_end_line = utils::Line(s->getLocEnd(), sm);
    clang::SourceLocation for_end_src_loc = sm.translateLineCol(sm.getMainFileID(), stmt_end_line + 1, 1);
    rewrite_pragma_.InsertText(for_end_src_loc, "}\n", true, false);
  }else {
    rewrite_pragma_.InsertText(stmt_start_src_loc, text.str(), true, true);
  }

  /* Comment the pragma */
  clang::SourceLocation pragma_src_loc = sm.translateLineCol(sm.getMainFileID(), stmt_start_line - 1, 1);
  rewrite_pragma_.InsertText(pragma_src_loc, "//", true, false);
  
/*
 * ----- Insert after pragma ----
 */
   
  std::stringstream text_after_pragma;
  text_after_pragma <<"\
void callme(ForParameter for_param) {\n";

  if(text_fx_var.str().compare("") == 0)
    text_after_pragma << "fx(for_param);\n";
  else
    text_after_pragma << "fx(for_param, " << text_fx_var.str() << ");\n";

text_after_pragma << 
"}\n\
};\n\
ThreadPool::getInstance(\"" << utils::FileName(s->getLocStart(), sm) 
    << "\")->call(std::make_shared<Nested>(" << n->getStartLine();
  
  if(text_constructor_var.str().compare("") != 0)
      text_after_pragma << ", ";
    
    text_after_pragma << text_constructor_var.str() <<"));\n\
}\n";

  unsigned stmt_end_line = utils::Line(s->getLocEnd(), sm);
  clang::SourceLocation pragma_end_src_loc = sm.translateLineCol(sm.getMainFileID(), stmt_end_line + 1, 1);

  rewrite_pragma_.InsertText(pragma_end_src_loc, text_after_pragma.str(), true, false);

}



Node *TransformRecursiveASTVisitor::GetNodeObjForPragma(clang::Stmt *s){

  clang::SourceLocation stmt_start_src_loc = s->getLocStart();
  unsigned stmt_start_line = utils::Line(stmt_start_src_loc, sm);

  std::vector<Root *>::iterator root_itr;
  for(root_itr = root_vect_->begin(); root_itr != root_vect_->end(); root_itr ++) {
    if((*root_itr)->getFunctionLineStart() < utils::Line(stmt_start_src_loc, sm) 
        && (*root_itr)->getFunctionLineEnd() > utils::Line(stmt_start_src_loc, sm))
      
      break;
  }
  
  std::vector<Node *>::iterator node_itr;
  Node * n;
  for(node_itr = (*root_itr)->children_vect_->begin(); 
      node_itr != (*root_itr)->children_vect_->end(); 
      node_itr ++) {
    
    n = RecursiveGetNodeObjforPragma(*node_itr, stmt_start_line);
    if(n != NULL)
      return n;
  }
  return NULL;
}

Node *TransformRecursiveASTVisitor::RecursiveGetNodeObjforPragma(Node *n, unsigned stmt_start_line) {
  Node *nn;
  if(n->getStartLine() == stmt_start_line){
        return n;
  }else if(n->children_vect_ != NULL) {
    for(std::vector<Node *>::iterator node_itr = n->children_vect_->begin(); 
        node_itr != n->children_vect_->end(); ++ node_itr) {
      
      nn = RecursiveGetNodeObjforPragma(*node_itr, stmt_start_line);
      if(nn != NULL)
        return nn;
    }
  }
  return NULL;
}


std::string TransformRecursiveASTVisitor::RewriteOMPFor(Node *n) {

  std::stringstream text_for;

  ForNode *for_node = n->for_node_;


  /* for( int i = a + for_param->thread_id_ *(b - a)/ num_threads_; .... */
  text_for << "for(" << for_node->loop_var_type_ << " " << for_node->loop_var_ << " = ";
  if(for_node->loop_var_init_val_set_)
    text_for << for_node->loop_var_init_val_;
  else
    text_for << for_node->loop_var_init_var_;

  text_for << " + for_param.thread_id_*(";
  if(for_node->condition_val_set_)
    text_for << for_node->condition_val_ << " - ";
  else
    text_for << for_node->condition_var_ << " - ";

  if(for_node->loop_var_init_val_set_)
    text_for << for_node->loop_var_init_val_;
  else
    text_for << for_node->loop_var_init_var_;

  text_for << ")/for_param.num_threads_; "; 


  /* ....; i < a + (for_param->thread_id_ + 1)*(b - a)/ num_threads_; ... */
  text_for << for_node->loop_var_ << " " << for_node->condition_op_ << " ";

  if(for_node->loop_var_init_val_set_)
    text_for << for_node->loop_var_init_val_;
  else
    text_for << for_node->loop_var_init_var_;

  text_for << " + (for_param.thread_id_ + 1)*(";
  if(for_node->condition_val_set_)
    text_for << for_node->condition_val_ << " - ";
  else
    text_for << for_node->condition_var_ << " - ";

  if(for_node->loop_var_init_val_set_)
    text_for << for_node->loop_var_init_val_;
  else
    text_for << for_node->loop_var_init_var_;

  text_for << ")/for_param.num_threads_; "; 
  

  /* ...; i ++) */
  text_for << for_node->loop_var_ << " " << for_node->increment_op_ << " ";
  if(for_node->increment_val_set_)
    text_for << for_node->increment_val_ << ")\n";
  else
    text_for << for_node->increment_var_ << ")\n";

  return text_for.str();

}