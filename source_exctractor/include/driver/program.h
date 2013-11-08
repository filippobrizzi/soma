#include "compiler.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"


class Program {

ClangCompiler ccompiler;
std::std::vector<clang::OMPExecutableDirective *> pragmaList;

void ParseSourceCode(std::string fileName);

public:
	Program(int argc, char **argv) : ccompiler(argc, argv) {}



	
};



class ProfilingRecursiveASTVisitor: public clang::RecursiveASTVisitor<ProfilingRecursiveASTVisitor> {

  clang::Rewriter &RewriteProfiling;
  clang::Rewriter &RewritePragma;

  std::vector<Node *> *nodeVect;
  
  const clang::SourceManager& sm;

  bool insertInclude;
  clang::Stmt *previousStmt;

  void RewriteProfile(clang::Stmt *s);
  std::string forCondition(const clang::Stmt *s);

public:
  ProfilingRecursiveASTVisitor(clang::Rewriter &RProfiling, clang::Rewriter &RPragma, std::vector<Node *> *nodeVect, const clang::SourceManager& sm) : 
          RewriteProfiling(RProfiling), RewritePragma(RPragma), nodeVect(nodeVect), sm(sm), insertInclude(false) previousStmt(NULL) { }
  
  bool VisitStmt(clang::Stmt *s);


  bool VisitFunctionDecl(clang::FunctionDecl *f);
    
};




class ProfilingASTConsumer : public clang::ASTConsumer {
 
 
public:

  ProfilingASTConsumer(clang::Rewriter &Rewrite, std::vector<Node *> *nodeVect, const clang::SourceManager& sm) : 
          rv(Rewrite, nodeVect, sm) { }
  
  virtual bool HandleTopLevelDecl(clang::DeclGroupRef d) {
    typedef clang::DeclGroupRef::iterator iter;
    for (iter b = d.begin(), e = d.end(); b != e; ++b) {
      rv.TraverseDecl(*b);
    } 
    return true; 
  }

  ProfilingRecursiveASTVisitor rv;
  std::vector<clang::OMPExecutableDirective *> pragmaList;

};

