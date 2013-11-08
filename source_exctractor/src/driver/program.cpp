
#include "program.h"

Program::Program(int argc, char **argv) {
	ccompiler(argc, argv);
	ParseSourceCode(argv[argc - 1]);
}


void Program::ParseSourceCode(std::string fileName) {
  	
  const FileEntry *pFile = compiler.getFileManager().getFile(fileName);
  compiler.getSourceManager().createMainFileID(pFile);
  compiler.getDiagnosticClient().BeginSourceFile(compiler.getLangOpts(), &compiler.getPreprocessor());

  // Convert <file>.c to <file_profile>.c
  std::string outNameProfile (fileName);
  size_t ext = outNameProfile.rfind(".");
  if (ext == std::string::npos)
      ext = outNameProfile.length();
  outNameProfile.insert(ext, "_profile");

  llvm::errs() << "Output to: " << outName << "\n";
  std::string OutErrorInfo;
  llvm::raw_fd_ostream outFileProfile(outNameProfile.c_str(), OutErrorInfo, 0);

  // Convert <file>.c to <file_pragma>.c
  std::string outNamePragma (fileName);
  size_t ext = outNamePragma.rfind(".");
  if (ext == std::string::npos)
      ext = outNamePragma.length();
  outNamePragma.insert(ext, "pragma");

  llvm::errs() << "Output to: " << outName << "\n";
  std::string OutErrorInfo;
  llvm::raw_fd_ostream outFilePragma(outNamePragma.c_str(), OutErrorInfo, 0);  


  clang::Rewriter rewriteProfiling;
  rewriteProfiling.setSourceMgr(compiler.getSourceManager(), compiler.getLangOpts());

  clang::Rewriter rewritePragma;
  rewritePragma.setSourceMgr(compiler.getSourceManager(), compiler.getLangOpts());


  ProfilingASTConsumer astConsumer(rewriteProfiling, rewritePragma, nodeVect, compiler.getSourceManager());
  
  // Parse the AST
  clang::ParseAST(compiler.getPreprocessor(), &astConsumer, compiler.getASTContext());

  compiler.getDiagnosticClient().EndSourceFile();


    // Now output rewritten source code
  const clang::RewriteBuffer *RewriteBufProfiling = rewriteProfiling.getRewriteBufferFor(compiler.getSourceManager().getMainFileID());
  outFileProfile << std::string(RewriteBufProfiling->begin(), RewriteBufProfiling->end());
  outFileProfile.close();
}


bool ProfilingRecursiveASTVisitor::VisitFunctionDecl(clang::FunctionDecl *f) {     
    
    clang::SourceLocation ST = f->getLocStart();
/*
 * ---- Include the path to ProfileTracker.h ----
 */
    if(this->insertInclude == false && sm.getFileID(ST) == sm.getMainFileID() && f->hasBody() == true ) {
      std::string text = "#include \"/home/pippo/Documents/Library/clomp-master/include/myprogram/profiling/ProfileTracker.h\"\n";
      Rewrite.InsertText(ST, text, true, false);
      this->insertInclude = true;
    }

/*
 * ---- Insert the call to the profilefunction tracker to track the execution time of each funcion ----
 * (l'if è necessario perchè non prenda in considerazione funzioni dichiarate negli include files)
 */
    if(sm.getFileID(ST) == sm.getMainFileID() && f->hasBody() == true) {
      unsigned startLine = clomp::utils::Line(ST, sm);
      clang::SourceLocation newSL = sm.translateLineCol(sm.getMainFileID(), startLine + 1, 1);
      std::stringstream text2;
      text2 << "if( ProfileTracker x = ProfileTrackParams(" << startLine << ", 0)) {\n";
      Rewrite.InsertText(newSL, text2.str(), true, false);

      clang::SourceLocation endSL = f->getLocEnd();
      std::stringstream text3;
      text3 << "}\n";
      Rewrite.InsertText(endSL, text3.str(), true, false);
    }
    return true; 
  }



bool ProfilingRecursiveASTVisitor::VisitStmt(clang::Stmt *s) {

	clang::SourceLocation ST = s->getLocStart();
	if(sm.getFileID(ST) == sm.getMainFileID()) {
  		if (clang::isa<clang::OMPExecutableDirective>(s) && s != ompexec) {
  			previousStmt = s;
  			pragmaList.insert(pragmaList.end(), static_cast<clang::OMPExecutableDirective *>(s));
			RewriteProfile(s);
  		}
  	}
  	
  return true;
}


void ProfilingRecursiveASTVisitor::RewriteProfile(clang::Stmt *s) {
	int fID = node->getParentFunctionInfo().parentFunctionLine;
  	int pragmaLine = node->sL.startLine;

      std::stringstream text;
      if(clang::isa<clang::ForStmt>(s)) {
        std::string conditionVar = forCondition(s);
        text << "if( ProfileTracker x = ProfileTrackParams(" << fID << ", " << pragmaLine << ", " << conditionVar << "))\n";
        Rewrite.InsertText(ST, text.str(), true, true);

      } else {
		    text << "if( ProfileTracker x = ProfileTrackParams(" << fID << ", " << pragmaLine << "))\n";
        Rewrite.InsertText(ST, text.str(), true, true);
      }

/*
 * ---- Comment the pragma ----
 */
      unsigned startLine = clomp::utils::Line(s->getLocStart(), sm);
      clang::SourceLocation pragmaST = sm.translateLineCol(sm.getMainFileID(), startLine - 1, 1);
      Rewrite.InsertText(pragmaST, "//", true, false);

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
}



void createAnnotatedCode(std::vector<Node *> *nodeVect) {

  // Convert <file>.c to <file_out>.c
  std::string outName (nodeVect->at(0)->sL.fileName);
  size_t ext = outName.rfind(".");
  if (ext == std::string::npos)
      ext = outName.length();
  outName.insert(ext, "_out");

  llvm::errs() << "Output to: " << outName << "\n";
  std::string OutErrorInfo;
  llvm::raw_fd_ostream outFile(outName.c_str(), OutErrorInfo, 0);


	clomp::ClangCompiler clangcomp(nodeVect->at(0)->sL.fileName);


	clang::Rewriter rewrite;
  rewrite.setSourceMgr(clangcomp.getSourceManager(), clangcomp.getLangOpts());

	ProfilingASTConsumer astConsumer(rewrite, nodeVect, clangcomp.getSourceManager());
  
  // Parse the AST
	clang::ParseAST(clangcomp.getPreprocessor(), &astConsumer, clangcomp.getASTContext());

  clangcomp.getDiagnosticClient().EndSourceFile();


    // Now output rewritten source code
  const clang::RewriteBuffer *RewriteBuf = rewrite.getRewriteBufferFor(clangcomp.getSourceManager().getMainFileID());

  outFile << std::string(RewriteBuf->begin(), RewriteBuf->end());
  
  outFile.close();

}

