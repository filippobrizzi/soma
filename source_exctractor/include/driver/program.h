#include "driver/compiler.h"
#include "utils/source_locations.h"

#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"

#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/Parse/Parser.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/raw_ostream.h"
#include <string>
#include <iostream>

class Program {

//ClangCompiler ccompiler;
std::vector<clang::OMPExecutableDirective *> *pragmaList;
std::vector<clang::FunctionDecl *> *functionList;


void ParseSourceCode(std::string fileName);

public:
	Program(int argc, char **argv) : ccompiler(argc, argv), pragmaList(NULL), functionList(NULL) {
		ParseSourceCode(argv[argc - 1]);
	}
	
	std::vector<clang::OMPExecutableDirective *> *getPragmaList() { return pragmaList; }
	std::vector<clang::FunctionDecl *> *getFunctionList() { return functionList; }

	ClangCompiler ccompiler;
	//clang::CompilerInstance ccompiler;
};



class ProfilingRecursiveASTVisitor: public clang::RecursiveASTVisitor<ProfilingRecursiveASTVisitor> {

  clang::Rewriter &RewriteProfiling;
  clang::Rewriter &RewritePragma;
  
  const clang::SourceManager& sm;

  bool insertInclude;
  clang::Stmt *previousStmt;

  void RewriteProfile(clang::Stmt *s);
  std::string forCondition(const clang::Stmt *s);
  unsigned getFunctionLine(clang::SourceLocation sl);

public:
  ProfilingRecursiveASTVisitor(clang::Rewriter &RProfiling, clang::Rewriter &RPragma, const clang::SourceManager& sm) : 
          RewriteProfiling(RProfiling), RewritePragma(RPragma), sm(sm), insertInclude(false), previousStmt(NULL) { }
  
  bool VisitStmt(clang::Stmt *s);
  bool VisitFunctionDecl(clang::FunctionDecl *f);

  std::vector<clang::OMPExecutableDirective *> pragmaList;
  std::vector<clang::FunctionDecl *> functionList;
    
};


class ProfilingASTConsumer : public clang::ASTConsumer { 
public:

  ProfilingASTConsumer(clang::Rewriter &RProfiling, clang::Rewriter &RPragma, const clang::SourceManager& sm) : 
          rv(RProfiling, RPragma, sm) { }
  
  virtual bool HandleTopLevelDecl(clang::DeclGroupRef d) {
    typedef clang::DeclGroupRef::iterator iter;
    for (iter b = d.begin(), e = d.end(); b != e; ++b) {
      rv.TraverseDecl(*b);
    } 
    return true; 
  }

  ProfilingRecursiveASTVisitor rv;
  std::vector<clang::OMPExecutableDirective *> pragmaList;
  std::vector<clang::FunctionDecl *> functionList;

};

