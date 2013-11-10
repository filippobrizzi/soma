
#include "driver/compiler.h"

using namespace clang;

ClangCompiler::ClangCompiler(int argc, char **argv) {

  DiagnosticOptions diagnosticOptions;
  compiler.createDiagnostics();

  // Create an invocation that passes any flags to preprocessor
  CompilerInvocation *Invocation = new CompilerInvocation;
  CompilerInvocation::CreateFromArgs(*Invocation, argv + 1, argv + argc,
                                      compiler.getDiagnostics());
  compiler.setInvocation(Invocation);

  // Set default target triple
  llvm::IntrusiveRefCntPtr<TargetOptions> pto( new TargetOptions());
  pto->Triple = llvm::sys::getDefaultTargetTriple();
  llvm::IntrusiveRefCntPtr<TargetInfo> pti(TargetInfo::CreateTargetInfo(compiler.getDiagnostics(), pto.getPtr()));
  compiler.setTarget(pti.getPtr());

  compiler.createFileManager();
  compiler.createSourceManager(compiler.getFileManager());

  HeaderSearchOptions &headerSearchOptions = compiler.getHeaderSearchOpts();

  headerSearchOptions.AddPath("/usr/local/include",
	          clang::frontend::Angled,
	          false,
	          false);

	headerSearchOptions.AddPath("/usr/include",
	          clang::frontend::Angled,
	          false,
	          false);

	headerSearchOptions.AddPath("/usr/lib/gcc/x86_64-linux-gnu/4.8/include",
	          clang::frontend::Angled,
	          false,
	          false);
	headerSearchOptions.AddPath("/home/pippo/Documents/Library/llvm_build/Release+Asserts/bin/../lib/clang/3.3.1/include",
	          clang::frontend::Angled,
	          false,
	          false);
	headerSearchOptions.AddPath("/usr/include/x86_64-linux-gnu",
	          clang::frontend::Angled,
	          false,
	          false);

 // Allow C++ code to get rewritten
  clang::LangOptions langOpts;
  langOpts.GNUMode = 1; 
  langOpts.CXXExceptions = 1; 
  langOpts.RTTI = 1; 
  langOpts.Bool = 1; 
  langOpts.CPlusPlus = 1; 
  Invocation->setLangDefaults(langOpts,
                              clang::IK_CXX,
                              clang::LangStandard::lang_cxx0x);

  compiler.createPreprocessor();
  compiler.getPreprocessorOpts().UsePredefines = false;

  compiler.createASTContext();

/*  	// Get filename
  	std::string fileName(argv[1]);  
  	const FileEntry *pFile = compiler.getFileManager().getFile(fileName);
  	compiler.getSourceManager().createMainFileID(pFile);
  	compiler.getDiagnosticClient().BeginSourceFile(compiler.getLangOpts(), &compiler.getPreprocessor());
*/

}
