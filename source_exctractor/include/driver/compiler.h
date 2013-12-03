#include "llvm/Support/Host.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"

#include "clang/Frontend/CompilerInstance.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/Lexer.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/AST/ASTContext.h"


//TODO make the class non copiable
/*
 * ---- Custom class to instantiate an object of clang::CompilerInstance with the options and the file
 * 		passed with argv.
 */
class ClangCompiler {

private:
	clang::CompilerInstance compiler_;

public:
	ClangCompiler(int argc, char **argv);
	
	//clang::CompilerInstance getCompilerInstance() { return compiler; }
	clang::SourceManager &getSourceManager() { return compiler.getSourceManager(); }
	clang::DiagnosticConsumer getDiagnosticClient() { return compiler.getDiagnosticClient(); }
	clang::LangOptions getLangOpts() { return compiler.getLangOpts(); }
	clang::Preprocessor &getPreprocessor() { return compiler.getPreprocessor(); }
	clang::ASTContext &getASTContext() { return compiler.getASTContext(); }
	clang::FileManager &getFileManager() { return compiler.getFileManager(); }

	//ProfilingASTConsumer astConsumer2;
};
