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

/*
 * ---- Instantiate a compiler object and start the parser. 
 */
class Program {
/* Contains the list of all the pragmas in the source code */
std::vector<clang::OMPExecutableDirective *> *pragma_list_;
/* Contains the list of all the functions defined in the source code (for profiling pourpuse) */
std::vector<clang::FunctionDecl *> *function_list_;

/* To create the profiling code and the list of pragmas */
void ParseSourceCode(std::string fileName);
/* To create the final source code to be used with the scheduler */
void ParseSourceCode(std::string fileName, std::vector<Root *> *root_vect);

public:
  /* To create the profiling code and the list of pragmas */
	Program(int argc, char **argv) : ccompiler_(argc, argv), pragma_list_(NULL), function_list_(NULL) {
		ParseSourceCode(argv[argc - 1]);
	}

  /* To create the final source code to be used with the scheduler */
  Program(int argc,char **argv, std::vector<Root *> *root_vect) : ccompiler(argc, argv), pragma_list_(NULL), function_list_(NULL) {
    ParseSourceCode(argv[argc - 1], root_vect);
  }
	
	std::vector<clang::OMPExecutableDirective *> *getPragmaList() { return pragma_list_; }
	std::vector<clang::FunctionDecl *> *getFunctionList() { return function_list_; }

	ClangCompiler ccompiler_;
};


/*
 * ---- Recursively visit the AST of the source code to exctract the pragmas and rewrite it
 *      adding profile call.
 */
class ProfilingRecursiveASTVisitor: public clang::RecursiveASTVisitor<ProfilingRecursiveASTVisitor> {

  /* Class to rewrite the code */
  clang::Rewriter &rewrite_profiling_;
  
  const clang::SourceManager& sm;

  bool include_inserted_;
  clang::Stmt *previous_stmt_;

  /* Add profiling call to a pragma stmt */
  void RewriteProfiling(clang::Stmt *s);
  /* Given a ForStmt retrieve the value of the condition variable, to know how many cycles will
     do the for */
  std::string ForConditionVarValue(const clang::Stmt *s);
  /* For a given stmt retrive the line of the function where it is defined */
  unsigned GetFunctionLineForPragma(clang::SourceLocation sl);

public:
  ProfilingRecursiveASTVisitor(clang::Rewriter &r_profiling, const clang::SourceManager& sm) : 
          RewriteProfiling(r_profiling), sm(sm), include_inserted_(false), previous_stmt_(NULL) { }
  
  /* This function is called for each stmt in the AST */
  bool VisitStmt(clang::Stmt *s);
  /* This function is called for each function in the AST */
  bool VisitFunctionDecl(clang::FunctionDecl *f);

  std::vector<clang::OMPExecutableDirective *> pragma_list_;
  std::vector<clang::FunctionDecl *> function_list_;
    
};

/*
 * ---- Is responible to call ProfilingRecurseASTVisitor.
 */
class ProfilingASTConsumer : public clang::ASTConsumer { 
public:

  ProfilingASTConsumer(clang::Rewriter &r_profiling, const clang::SourceManager& sm) : 
          rv(r_profiling, sm) { }
  
  /* Traverse the AST invoking the RecursiveASTVisitor functions */
  virtual bool HandleTopLevelDecl(clang::DeclGroupRef d) {
    typedef clang::DeclGroupRef::iterator iter;
    for (iter b = d.begin(), e = d.end(); b != e; ++b) {
      recursive_visitor.TraverseDecl(*b);
    } 
    return true; 
  }

  ProfilingRecursiveASTVisitor recursive_visitor;
  std::vector<clang::OMPExecutableDirective *> pragma_list_;
  std::vector<clang::FunctionDecl *> function_list_;
};

/*
 * --------------------------------------------------------------------------------------------------
 * --------------------------------------------------------------------------------------------------
 */


/*
 * ---- Recursively visit the AST and repleace each pragma with a function call.
 */
class TransformRecursiveASTVisitor: public clang::RecursiveASTVisitor<TransformRecursiveASTVisitor> {

  clang::Rewriter &rewrite_pragma_;
  
  const clang::SourceManager& sm;

  /* Needed because the parse retrive twice each pragma stmt */
  clang::Stmt *previous_stmt_;
  /* Check if the inlude command has been already inserted*/
  bool include_inserted_;

  std::vector<Root *> *root_vect_;

  void RewriteOMPPragma(clang::Stmt *associated_stmt);
  void RewriteOMPBarrier(clang::OMPExecutableDirective *omp_stmt);
  std::string RewriteOMPFor(Node *n);
  
  /* Given a pragma stmt retrive the Node object that contains all its info */
  Node *GetNodeObjForPragma(clang::Stmt *s);
  /* Called by GetNodeObjForPragma is used because the Node objs are saved in a tree */
  Node *RecursiveGetNodeObjforPragma(Node *n, unsigned stmt_start_line);


public:
  TransformRecursiveASTVisitor(clang::Rewriter &r_pragma_, std::vector<Root *> *root_vect, const clang::SourceManager& sm) : 
          RewritePragma(r_pragma_), root_vect_(root_vect), sm(sm), include_inserted_(false), previous_stmt_(NULL) { }
  
  bool VisitStmt(clang::Stmt *s);
  bool VisitFunctionDecl(clang::FunctionDecl *f);
};

/*
 * ---- Responsible to invoke TransformRecursiveASTVisitor.
 */
class TransformASTConsumer : public clang::ASTConsumer { 
public:

  TransformASTConsumer(clang::Rewriter &RPragma, std::vector<Root *> *rootVect, const clang::SourceManager& sm) : 
          rv(RPragma, rootVect, sm) { }
  
  virtual bool HandleTopLevelDecl(clang::DeclGroupRef d) {
    typedef clang::DeclGroupRef::iterator iter;
    for (iter b = d.begin(), e = d.end(); b != e; ++b) {
      recursive_visitor.TraverseDecl(*b);
    } 
    return true; 
  }

  TransformRecursiveASTVisitor recursive_visitor;
};