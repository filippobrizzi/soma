
#include "driver/program.h"


void Program::ParseSourceCode(std::string fileName) {

  // Convert <file>.c to <file_profile>.c
  std::string outNameProfile (fileName);
  size_t ext = outNameProfile.rfind(".");
  if (ext == std::string::npos)
  	ext = outNameProfile.length();
  outNameProfile.insert(ext, "_profile");

  llvm::errs() << "Output to: " << outNameProfile << "\n";
  std::string OutErrorInfo;
  llvm::raw_fd_ostream outFileProfile(outNameProfile.c_str(), OutErrorInfo, 0);

  // Convert <file>.c to <file_pragma>.c
/*	std::string outNamePragma (fileName);
	ext = outNamePragma.rfind(".");
	if (ext == std::string::npos)
 		ext = outNamePragma.length();
	outNamePragma.insert(ext, "_pragma");

	llvm::errs() << "Output to: " << outNamePragma << "\n";
	llvm::raw_fd_ostream outFilePragma(outNamePragma.c_str(), OutErrorInfo, 0);  
*/
  clang::Rewriter rewriteProfiling;
  rewriteProfiling.setSourceMgr(ccompiler.getSourceManager(), ccompiler.getLangOpts());
  clang::Rewriter rewritePragma;
  rewritePragma.setSourceMgr(ccompiler.getSourceManager(), ccompiler.getLangOpts());


  ProfilingASTConsumer astConsumer(rewriteProfiling, rewritePragma, ccompiler.getSourceManager());
  // Parse the AST
  clang::ParseAST(ccompiler.getPreprocessor(), &astConsumer, ccompiler.getASTContext());
	ccompiler.getDiagnosticClient().EndSourceFile();

	this->pragmaList = new std::vector<clang::OMPExecutableDirective *>(astConsumer.rv.pragmaList);
	this->functionList = new std::vector<clang::FunctionDecl *>(astConsumer.rv.functionList);


  // Now output rewritten source code
	const clang::RewriteBuffer *RewriteBufProfiling = rewriteProfiling.getRewriteBufferFor(ccompiler.getSourceManager().getMainFileID());  	
  outFileProfile << std::string(RewriteBufProfiling->begin(), RewriteBufProfiling->end());

  outFileProfile.close();

  //const clang::RewriteBuffer *RewriteBufPragma = rewritePragma.getRewriteBufferFor(ccompiler.getSourceManager().getMainFileID());
  //outFilePragma << std::string(RewriteBufPragma->begin(), RewriteBufPragma->end());
  //outFilePragma.close();
  
}




bool ProfilingRecursiveASTVisitor::VisitFunctionDecl(clang::FunctionDecl *f) {     
    
    clang::SourceLocation ST = f->getLocStart();

/*
 * ---- Insert the call to the profilefunction tracker to track the execution time of each funcion ----
 * (l'if è necessario perchè non prenda in considerazione funzioni dichiarate negli include files)
 */
  if(sm.getFileID(ST) == sm.getMainFileID() && f->hasBody() == true) {
  	
    functionList.insert(functionList.end(), f);
/*
 * ---- Include the path to ProfileTracker.h ----
 */
    if(this->insertInclude == false) {
      std::string text = "#include \"/home/pippo/Documents/Library/clomp-master/include/myprogram/profiling/ProfileTracker.h\"\n";
      RewriteProfiling.InsertText(ST, text, true, false);
      this->insertInclude = true;
    } 

    ST = f->getBody()->getLocStart();
  	unsigned startLine = utils::Line(ST, sm);
    clang::SourceLocation newSL = sm.translateLineCol(sm.getMainFileID(), startLine + 1, 1);
    std::stringstream text2;
    text2 << "if( ProfileTracker x = ProfileTrackParams(" << startLine << ", 0)) {\n";
    RewriteProfiling.InsertText(newSL, text2.str(), true, false);

    clang::SourceLocation endSL = f->getLocEnd();
    std::stringstream text3;
    text3 << "}\n";
    RewriteProfiling.InsertText(endSL, text3.str(), true, false);

    std::cout << f->getNameInfo().getAsString() << " - " << utils::Line(f->getLocStart(), sm) << std::endl;
  }
  return true; 
}



bool ProfilingRecursiveASTVisitor::VisitStmt(clang::Stmt *s) {

	clang::SourceLocation ST = s->getLocStart();
	if(sm.getFileID(ST) == sm.getMainFileID()) {
  		if (clang::isa<clang::OMPExecutableDirective>(s) && s != previousStmt) {
  			previousStmt = s;
  			clang::OMPExecutableDirective *omps = static_cast<clang::OMPExecutableDirective *>(s);
  			pragmaList.insert(pragmaList.end(), omps);
        std::cout << "Pragma " << omps->getStmtClassName() << " - " << utils::Line(omps->getLocStart(), sm) << " - clauses: " << omps->getNumClauses() << std::endl;
  			clang::Stmt *as = omps->getAssociatedStmt();
        if(as) {
          clang::Stmt *cs = static_cast<clang::CapturedStmt *>(as)->getCapturedStmt();

/*
 * ---- In the case of #omp parallel for we have to go down two level befor finding the ForStmt ----
 */
  		    if(strcmp(cs->getStmtClassName(), "OMPForDirective") != 0)
            RewriteProfile(cs);
        
        }        
      }
  	}
  	
  return true;
}


void ProfilingRecursiveASTVisitor::RewriteProfile(clang::Stmt *s) {
  	
  	clang::SourceLocation ST = s->getLocStart();
  	unsigned pragmaLine = utils::Line(ST,sm);
  	unsigned functionLine = getFunctionLine(s->getLocStart());

    std::stringstream text;
    if(clang::isa<clang::ForStmt>(s)) {
      //std::string conditionVar = forCondition(s);
      std::string conditionVar = "";
      text << "if( ProfileTracker x = ProfileTrackParams(" << functionLine << ", " << pragmaLine << ", " << conditionVar << "))\n";
      RewriteProfiling.InsertText(ST, text.str(), true, true);

    } else {
	    text << "if( ProfileTracker x = ProfileTrackParams(" << functionLine << ", " << pragmaLine << "))\n";
      RewriteProfiling.InsertText(ST, text.str(), true, true);
    }

/*
 * ---- Comment the pragma ----
 */
    unsigned startLine = utils::Line(s->getLocStart(), sm);
    clang::SourceLocation pragmaST = sm.translateLineCol(sm.getMainFileID(), startLine - 1, 1);
    RewriteProfiling.InsertText(pragmaST, "//", true, false);

}




std::string ProfilingRecursiveASTVisitor::forCondition(const clang::Stmt *s) {
	const clang::ForStmt *fS = static_cast<const clang::ForStmt *>(s);
  	const clang::Expr *cE = fS->getCond();
  	const clang::BinaryOperator *bO = static_cast<const clang::BinaryOperator *>(cE);

/*
 *  Conditional value
 */
  	const clang::Expr *rEx = bO->getRHS();
  	if(strcmp(rEx->getStmtClassName(), "IntegerLiteral") == 0) {
    	const clang::IntegerLiteral *iL = static_cast<const clang::IntegerLiteral *>(rEx);
    	std::stringstream text;
    	text << iL->getValue().getZExtValue();
    	return text.str();

  	} else if(strcmp(rEx->getStmtClassName(), "ImplicitCastExpr") == 0) {
    	const clang::DeclRefExpr *dRE = static_cast<const clang::DeclRefExpr *>(*(rEx->child_begin()));
    	const clang::NamedDecl *nD = dRE->getFoundDecl();
    	return nD->getNameAsString(); 
  	}
  	return "";
}

unsigned ProfilingRecursiveASTVisitor::getFunctionLine(clang::SourceLocation sl) {

	unsigned pragmaLine = utils::Line(sl, sm);
	unsigned startFuncLine, endFuncLine;
	std::vector<clang::FunctionDecl *>::iterator fitr;
	for(fitr = functionList.begin(); fitr != functionList.end(); ++ fitr) {
		startFuncLine = utils::Line((*fitr)->getSourceRange().getBegin(), sm);
		endFuncLine = utils::Line((*fitr)->getSourceRange().getEnd(), sm);
		if(pragmaLine < endFuncLine && pragmaLine > startFuncLine)
			return startFuncLine;
	}

	return 0;
}

