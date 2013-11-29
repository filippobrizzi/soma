#include "driver/compiler.h"
#include "utils/source_locations.h"
#include "pragma_handler/Root.h"

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

std::vector<clang::OMPExecutableDirective *> *pragmaList;
std::vector<clang::FunctionDecl *> *functionList;


void ParseSourceCode(std::string fileName);
void ParseSourceCode(std::string fileName, std::vector<Root *> *rootVect);

public:
	Program(int argc, char **argv) : ccompiler(argc, argv), pragmaList(NULL), functionList(NULL) {
		ParseSourceCode(argv[argc - 1]);
	}

  Program(int argc,char **argv, std::vector<Root *> *rootVect) : ccompiler(argc, argv), pragmaList(NULL), functionList(NULL) {
    ParseSourceCode(argv[argc - 1], rootVect);
  }
	
	std::vector<clang::OMPExecutableDirective *> *getPragmaList() { return pragmaList; }
	std::vector<clang::FunctionDecl *> *getFunctionList() { return functionList; }

	ClangCompiler ccompiler;
};



class ProfilingRecursiveASTVisitor: public clang::RecursiveASTVisitor<ProfilingRecursiveASTVisitor> {

  clang::Rewriter &RewriteProfiling;
  
  const clang::SourceManager& sm;

  bool insertInclude;
  clang::Stmt *previousStmt;

  void RewriteProfile(clang::Stmt *s);
  std::string forCondition(const clang::Stmt *s);
  unsigned getFunctionLine(clang::SourceLocation sl);
  //clang::FunctionDecl *EmitFunction(clang::Stmt *s);

public:
  ProfilingRecursiveASTVisitor(clang::Rewriter &RProfiling, const clang::SourceManager& sm) : 
          RewriteProfiling(RProfiling), sm(sm), insertInclude(false), previousStmt(NULL) { }
  
  bool VisitStmt(clang::Stmt *s);
  bool VisitFunctionDecl(clang::FunctionDecl *f);

  std::vector<clang::OMPExecutableDirective *> pragmaList;
  std::vector<clang::FunctionDecl *> functionList;
    
};


class ProfilingASTConsumer : public clang::ASTConsumer { 
public:

  ProfilingASTConsumer(clang::Rewriter &RProfiling, const clang::SourceManager& sm) : 
          rv(RProfiling, sm) { }
  
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

/*
 * --------------------------------------------------------------------------------------------------
 * --------------------------------------------------------------------------------------------------
 */

class TransformRecursiveASTVisitor: public clang::RecursiveASTVisitor<TransformRecursiveASTVisitor> {

  clang::Rewriter &RewritePragma;
  
  const clang::SourceManager& sm;

  clang::Stmt *previousStmt;
  bool insertInclude;

  std::vector<Root *> *rootVect;

  void RewriteOMP(clang::Stmt *s);
  Node *getNodeforPragma(clang::Stmt *s);
  Node *recursiveNodeforPragma(Node *n, unsigned l);
  std::string GetParallelFor(Node *n);

public:
  TransformRecursiveASTVisitor(clang::Rewriter &RPragma, std::vector<Root *> *rootVect, const clang::SourceManager& sm) : 
          RewritePragma(RPragma), rootVect(rootVect), sm(sm), insertInclude(false), previousStmt(NULL) { }
  
  bool VisitStmt(clang::Stmt *s);
  bool VisitFunctionDecl(clang::FunctionDecl *f);
};


class TransformASTConsumer : public clang::ASTConsumer { 
public:

  TransformASTConsumer(clang::Rewriter &RPragma, std::vector<Root *> *rootVect, const clang::SourceManager& sm) : 
          rv(RPragma, rootVect, sm) { }
  
  virtual bool HandleTopLevelDecl(clang::DeclGroupRef d) {
    typedef clang::DeclGroupRef::iterator iter;
    for (iter b = d.begin(), e = d.end(); b != e; ++b) {
      rv.TraverseDecl(*b);
    } 
    return true; 
  }

  TransformRecursiveASTVisitor rv;
};