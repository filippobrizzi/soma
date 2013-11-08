#include "clang/Frontend/CompilerInstance.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/Lexer.h"
#include "clang/Basic/Diagnostic.h"


//TODO make the class non copiable
class ClangCompiler {

private:
	clang::CompilerInstance compiler;

public:
	ClangCompiler(int argc, char **argv);
	
	clang::CompilerInstance getCompilerInstance() { return compiler; }

};





