
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

  clang::Rewriter rewriteProfiling;
  rewriteProfiling.setSourceMgr(ccompiler.getSourceManager(), ccompiler.getLangOpts());
  
  ProfilingASTConsumer astConsumer(rewriteProfiling, ccompiler.getSourceManager());
  // Parse the AST
  clang::ParseAST(ccompiler.getPreprocessor(), &astConsumer, ccompiler.getASTContext());
	ccompiler.getDiagnosticClient().EndSourceFile();

	this->pragmaList = new std::vector<clang::OMPExecutableDirective *>(astConsumer.rv.pragmaList);
	this->functionList = new std::vector<clang::FunctionDecl *>(astConsumer.rv.functionList);


  // Now output rewritten source code
	const clang::RewriteBuffer *RewriteBufProfiling = rewriteProfiling.getRewriteBufferFor(ccompiler.getSourceManager().getMainFileID());  	
  outFileProfile << std::string(RewriteBufProfiling->begin(), RewriteBufProfiling->end());
  outFileProfile.close();


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
  			clang::Stmt *as = omps->getAssociatedStmt();
        if(as) {
          clang::Stmt *cs = static_cast<clang::CapturedStmt *>(as)->getCapturedStmt();
          //In the case of #omp parallel for we have to go down two level befor finding the ForStmt 
  		    if(strcmp(cs->getStmtClassName(), "OMPForDirective") != 0)
            
            RewriteProfile(cs);

          std::cout << s->getStmtClassName() << " " << utils::Line(s->getLocStart() ,sm) << std::endl;
          clang::CapturedStmt *cs2 = static_cast<clang::CapturedStmt *>(as);
          for(clang::CapturedStmt::capture_iterator I = cs2->capture_begin(); I != cs2->capture_end(); ++I){
            clang::VarDecl *vd = I->getCapturedVar(); 
            std::cout << vd->getNameAsString() << " - " << vd->getType().getAsString() << std::endl;
          }
          std::cout << std::endl;
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
      std::string conditionVar = forCondition(s);
      //std::string conditionVar = "";
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
  std::string startValue, endValue;

/*
 *  Conditional end value
 */
  const clang::Expr *rEx = bO->getRHS();
  if(strcmp(rEx->getStmtClassName(), "IntegerLiteral") == 0) {
    const clang::IntegerLiteral *iL = static_cast<const clang::IntegerLiteral *>(rEx);
    std::stringstream text;
    text << iL->getValue().getZExtValue();
    //return text.str();
    endValue = text.str();

  } else if(strcmp(rEx->getStmtClassName(), "ImplicitCastExpr") == 0) {
  	const clang::DeclRefExpr *dRE = static_cast<const clang::DeclRefExpr *>(*(rEx->child_begin()));
  	const clang::NamedDecl *nD = dRE->getFoundDecl();
  	//return nD->getNameAsString(); 
    endValue = nD->getNameAsString();
  }

/*
 * Conditional start value
 */

  if(strcmp(fS->child_begin()->getStmtClassName(), "DeclStmt") == 0) {
    const clang::DeclStmt *dS = static_cast<const clang::DeclStmt *>(*(fS->child_begin()));
    const clang::Decl *d = dS->getSingleDecl();
/*
 *  for (... = 0)
 */
    if(strcmp(dS->child_begin()->getStmtClassName(), "IntegerLiteral") == 0) {      
      const clang::IntegerLiteral *iL = static_cast<const clang::IntegerLiteral *>(*(dS->child_begin())); 
      std::stringstream text;
      text << iL->getValue().getZExtValue();
      //return text.str();
      startValue = text.str();/*
 *  for (... = a)
 */
    }else if (strcmp(dS->child_begin()->getStmtClassName(), "ImplicitCastExpr") == 0) {
      const clang::DeclRefExpr *dRE = static_cast<const clang::DeclRefExpr *>(*(dS->child_begin()->child_begin()));
      const clang::NamedDecl *nD = dRE->getFoundDecl();
      startValue = nD->getNameAsString();
    }
  }
/*
 *  for ( i = ...)
 */
  else if(strcmp(fS->child_begin()->getStmtClassName(), "BinaryOperator") == 0) {
    const clang::BinaryOperator *bO = static_cast<const clang::BinaryOperator *>(*(fS->child_begin())); 
    const clang::DeclRefExpr *dRE = static_cast<const clang::DeclRefExpr *>(*(bO->child_begin()));
/*
 *  for( ... = 0)
 */
    clang::ConstStmtIterator stI = bO->child_begin();
    stI ++;
    if(strcmp(stI->getStmtClassName(), "IntegerLiteral") == 0) {
      const clang::IntegerLiteral *iL = static_cast<const clang::IntegerLiteral *>(*stI);
      startValue = iL->getValue().getZExtValue();      
/*
 *  for ( ... = a)
 */
    } else if (strcmp(stI->getStmtClassName(), "ImplicitCastExpr") == 0) {
      const clang::DeclRefExpr *dRE = static_cast<const clang::DeclRefExpr *>(*(stI->child_begin()));
      const clang::NamedDecl *nD = dRE->getFoundDecl();
      startValue = nD->getNameAsString();
    }
  }
  endValue.append(" - ");
  endValue.append(startValue);
  return endValue;
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
/*
 * -------------------------------------------------------------------------------------------------------------------
 * -------------------------------------------------------------------------------------------------------------------
 */

void Program::ParseSourceCode(std::string fileName, std::vector<Root *> *rootVect) {

 // Convert <file>.c to <file_transformed>.c
  std::string outNamePragma (fileName);
  size_t ext = outNamePragma.rfind(".");
  if (ext == std::string::npos)
    ext = outNamePragma.length();
  outNamePragma.insert(ext, "_tranformed");

  llvm::errs() << "Output to: " << outNamePragma << "\n";
  std::string OutErrorInfo;
  llvm::raw_fd_ostream outFilePragma(outNamePragma.c_str(), OutErrorInfo, 0);  

  clang::Rewriter rewritePragma;
  rewritePragma.setSourceMgr(ccompiler.getSourceManager(), ccompiler.getLangOpts());

  TransformASTConsumer tastConsumer(rewritePragma, rootVect, ccompiler.getSourceManager());
  
  // Parse the AST

  clang::ParseAST(ccompiler.getPreprocessor(), &tastConsumer, ccompiler.getASTContext());
  ccompiler.getDiagnosticClient().EndSourceFile();

  const clang::RewriteBuffer *RewriteBufPragma = rewritePragma.getRewriteBufferFor(ccompiler.getSourceManager().getMainFileID());
  outFilePragma << std::string(RewriteBufPragma->begin(), RewriteBufPragma->end());
  outFilePragma.close();
}


bool TransformRecursiveASTVisitor::VisitFunctionDecl(clang::FunctionDecl *f) {     
  clang::SourceLocation ST = f->getLocStart();

  if(sm.getFileID(ST) == sm.getMainFileID() && !clang::isa<clang::CXXMethodDecl>(f)) {
    if(this->insertInclude == false) {
      this->insertInclude = true;

      std::string text = "#include \"instancerun.h\"\n";
/*\n\
class ForParameter\n\
{\n\
public:\n\
  int tid;\n\
  int numThread;\n\
  ForParameter(int tid, int numThread) : tid(tid), numThread(numThread) {}\n\
};\n\
\n\
class NestedBase { \n\
public: \n\
  ForParameter *fp;\n\
  int pragmaID;\n\
  virtual void callme() = 0;\n\
  void operator()() {\n\
    callme();\n\
  }\n\
};\n\
\n";
*/

      RewritePragma.InsertText(ST, text, true, false);
    }
  }


  return true;
}

bool TransformRecursiveASTVisitor::VisitStmt(clang::Stmt *s) {
  
  clang::SourceLocation ST = s->getLocStart();
  if(sm.getFileID(ST) == sm.getMainFileID()) {
    if (clang::isa<clang::OMPExecutableDirective>(s) && s != previousStmt) {
      previousStmt = s;
      clang::OMPExecutableDirective *omps = static_cast<clang::OMPExecutableDirective *>(s);
      clang::Stmt *as = omps->getAssociatedStmt();
      if(as) {
        clang::Stmt *cs = static_cast<clang::CapturedStmt *>(as)->getCapturedStmt();
        if(strcmp(cs->getStmtClassName(), "OMPForDirective") != 0) {
          RewriteOMP(as);
        }
      }else if(strcmp(omps->getStmtClassName(), "OMPBarrierDirective") == 0){
        RewriteBarrier(omps);
      }
    }
  }
  return true;
}


void TransformRecursiveASTVisitor::RewriteBarrier(clang::OMPExecutableDirective *omps) {
  unsigned startLine = utils::Line(omps->getLocStart(), sm);
  std::stringstream text;
  text <<
"{\n\
  class Nested : public NestedBase {\n\
  public: \n\
    Nested(int pragmaID) : NestedBase(pragmaID) {}\n\
    void callme(){}\n\
  };\n\
  Nested _x_(" << startLine << ");\n\
  InstanceRun::getInstance(\"" << utils::FileName(omps->getLocStart(), sm) << "\")->call(_x_);\n\
}";

  clang::SourceLocation pragmaST = sm.translateLineCol(sm.getMainFileID(), startLine + 1, 1);
  RewritePragma.InsertText(pragmaST, text.str(), true, false);

  pragmaST = sm.translateLineCol(sm.getMainFileID(), startLine, 1);
  RewritePragma.InsertText(pragmaST, "//", true, false);
}


void TransformRecursiveASTVisitor::RewriteOMP(clang::Stmt *as) {
  
  clang::Stmt *s = static_cast<clang::CapturedStmt *>(as)->getCapturedStmt();

  clang::SourceLocation ST = s->getLocStart();
  unsigned pragmaLine = utils::Line(ST,sm);

  Node *n = getNodeforPragma(s);
  
  std::stringstream text;
  std::stringstream textParam;
  std::stringstream textVar;
  std::stringstream textCallmeVar;
  std::stringstream textNested;
  std::stringstream textVarConstr;
/*
 * ----- Insert before pragma ----
 */
  text <<
"{\n\
  class Nested : public NestedBase {\n\
  public: \n\
    Nested(int pragmaID, ";

  textVarConstr << " : NestedBase(pragmaID), ";

  clang::CapturedStmt *cs = static_cast<clang::CapturedStmt *>(as);

  for(clang::CapturedStmt::capture_iterator I = cs->capture_begin(); I != cs->capture_end(); ++I){
    clang::VarDecl *vd = I->getCapturedVar(); 
    std::string type = vd->getType().getAsString();

    if(I != cs->capture_begin()){
      textParam << ", ";
      textCallmeVar << ", ";
      textNested << ", ";
    }

    if(type.find("class") != std::string::npos)
      type.erase(0, 6);

    if(n->optionVect->find("private") != n->optionVect->end()) {
      if(n->optionVect->find("private")->second.find(vd->getNameAsString()) != n->optionVect->find("private")->second.end() 
            || type.find("*") != std::string::npos){

        textParam << type << " " << vd->getNameAsString();
        textVar << type << " " << vd->getNameAsString() << "_;\n";

      }else{
        textParam << type << " & " << vd->getNameAsString();
        textVar << type << " & " << vd->getNameAsString() << "_;\n";
      }
    }else if(type.find("*") != std::string::npos) {
      textParam << type << " " << vd->getNameAsString();
      textVar << type << " " << vd->getNameAsString() << "_;\n";

    }else {
      textParam << type << " & " << vd->getNameAsString();
      textVar << type << " & " << vd->getNameAsString() << "_;\n";
    }

    textVarConstr << vd->getNameAsString() << "_(" << vd->getNameAsString() << ") ";
    textCallmeVar << vd->getNameAsString() << "_";
    textNested << vd->getNameAsString();
  }

  text << textParam.str() << ") " << textVarConstr.str() << "{}\n" << textVar.str() << "\n";    
  
  unsigned startLine = utils::Line(s->getLocStart(), sm);
  if(n->forNode == NULL) {
    text << "void fx(" << textParam.str() <<")\n";
    RewritePragma.InsertText(ST, text.str(), true, true);

  }else {
    std::string textFor;
    textFor = GetParallelFor(n);

    text << "void fx(" << textParam.str() <<") {\n" << textFor;

    clang::SourceLocation forST = sm.translateLineCol(sm.getMainFileID(), startLine + 1, 1);
    RewritePragma.InsertText(forST, text.str(), true, false);
    RewritePragma.InsertText(ST, "//", true, false);

    RewritePragma.InsertText(s->getLocEnd(), "}\n", true, false);
  }

  clang::SourceLocation pragmaST = sm.translateLineCol(sm.getMainFileID(), startLine - 1, 1);
  RewritePragma.InsertText(pragmaST, "//", true, false);
  
/*
 * ----- Insert after pragma ----
 */
   
  std::stringstream textAfter;
  textAfter <<"\
void callme() {\n\
  fx(" << textCallmeVar.str() << ");\n\
}\n\
};\n\
Nested *_x_ = new Nested(" << n->getStartLine();
  if(textNested.str().compare("") != 0)
    textAfter << ", ";
  textAfter << textNested.str() <<");\n\
InstanceRun::getInstance(\"" << utils::FileName(s->getLocStart(), sm) << "\")->call(*_x_);\n\
}\n";

  unsigned endLine = utils::Line(s->getLocEnd(), sm);
  clang::SourceLocation pragmaEndST = sm.translateLineCol(sm.getMainFileID(), endLine + 1, 1);

  RewritePragma.InsertText(pragmaEndST, textAfter.str(), true, false);


}



Node *TransformRecursiveASTVisitor::getNodeforPragma(clang::Stmt *s){

  clang::SourceLocation ST = s->getLocStart();
  unsigned l = utils::Line(ST, sm);

  std::vector<Root *>::iterator ritr;
  for(ritr = rootVect->begin(); ritr != rootVect->end(); ritr ++) {
    if((*ritr)->getFunctionLineStart() < utils::Line(ST, sm) && (*ritr)->getFunctionLineEnd() > utils::Line(ST, sm))
      break;
  }
  
  std::vector<Node *>::iterator nitr;
  Node * n;
  for(nitr = (*ritr)->childrenVect->begin(); nitr != (*ritr)->childrenVect->end(); nitr ++) {
    n = recursiveNodeforPragma(*nitr, l);
    if(n != NULL)
      return n;
  }
  return NULL;
}

Node *TransformRecursiveASTVisitor::recursiveNodeforPragma(Node *n, unsigned l) {
  Node *nn;
  if(n->getStartLine() == l){
        return n;
  }else if(n->childrenVect != NULL) {
    for(std::vector<Node *>::iterator nitr = n->childrenVect->begin(); nitr != n->childrenVect->end(); ++ nitr) {
      nn = recursiveNodeforPragma(*nitr, l);
      if(nn != NULL)
        return nn;
    }
  }
  return NULL;
}


std::string TransformRecursiveASTVisitor::GetParallelFor(Node *n) {

  std::stringstream text;

  ForNode *fn = n->forNode;


//for( int i = a + fp->tid *(b - a)/ numThread; ....
  text << "for(" << fn->loopVarType << " " << fn->loopVar << " = ";
  if(fn->loopVarInitValSet)
    text << fn->loopVarInitVal;
  else
    text << fn->loopVarInitVar;

  text << " + fp->tid*(";
  if(fn->conditionValSet)
    text << fn->conditionVal << " - ";
  else
    text << fn->conditionVar << " - ";

  if(fn->loopVarInitValSet)
    text << fn->loopVarInitVal;
  else
    text << fn->loopVarInitVar;

  text << ")/fp->numThread; "; 


  // ....; i < a + (fp->tid + 1)*(b - a)/ numThread; ...
  text << fn->loopVar << " " << fn->conditionOp << " ";

  if(fn->loopVarInitValSet)
    text << fn->loopVarInitVal;
  else
    text << fn->loopVarInitVar;

  text << " + (fp->tid + 1)*(";
  if(fn->conditionValSet)
    text << fn->conditionVal << " - ";
  else
    text << fn->conditionVar << " - ";

  if(fn->loopVarInitValSet)
    text << fn->loopVarInitVal;
  else
    text << fn->loopVarInitVar;

  text << ")/fp->numThread; "; 
  

  // ...; i ++)
  text << fn->loopVar << " " << fn->incrementOp << " ";
  if(fn->incrementValSet)
    text << fn->incrementVal << ")\n";
  else
    text << fn->incrementVar << ")\n";

  return text.str();

}