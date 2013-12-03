
#include "driver/compiler.h"

using namespace clang;

ClangCompiler::ClangCompiler(int argc, char **argv) {

  DiagnosticOptions diagnosticOptions;
  compiler_.createDiagnostics();

  /* Create an invocation that passes any flags to preprocessor */
  CompilerInvocation *Invocation = new CompilerInvocation;
  CompilerInvocation::CreateFromArgs(*Invocation, argv + 1, argv + argc,
                                      compiler_.getDiagnostics());
  compiler_.setInvocation(Invocation);

  /* Set default target triple */
  llvm::IntrusiveRefCntPtr<TargetOptions> pto( new TargetOptions());
  pto->Triple = llvm::sys::getDefaultTargetTriple();
  llvm::IntrusiveRefCntPtr<TargetInfo> pti(TargetInfo::CreateTargetInfo(compiler_.getDiagnostics(), pto.getPtr()));
  compiler_.setTarget(pti.getPtr());

  compiler_.createFileManager();
  compiler_.createSourceManager(compiler_.getFileManager());

  /* Add default search path for the compiler */
  HeaderSearchOptions &headerSearchOptions = compiler_.getHeaderSearchOpts();

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
  headerSearchOptions.AddPath("/usr/include/c++/4.8/",
            clang::frontend::Angled,
            false,
            false);

  headerSearchOptions.AddPath("/usr/include/x86_64-linux-gnu/c++/4.8/",
            clang::frontend::Angled,
            false,
            false);


  /* Allow C++ code to get rewritten */
  clang::LangOptions langOpts;
  langOpts.GNUMode = 1; 
  langOpts.CXXExceptions = 1; 
  langOpts.RTTI = 1; 
  langOpts.Bool = 1; 
  langOpts.CPlusPlus = 1; 
  Invocation->setLangDefaults(langOpts,
                              clang::IK_CXX,
                              clang::LangStandard::lang_cxx0x);

  compiler_.createPreprocessor();
  compiler_.getPreprocessorOpts().UsePredefines = false;

  compiler_.createASTContext();

  /* Initialize the compiler and the source manager with a file to process */
  std::string fileName(argv[argc - 1]);  
  const FileEntry *pFile = compiler_.getFileManager().getFile(fileName);
  compiler_.getSourceManager().createMainFileID(pFile);
  compiler_.getDiagnosticClient().BeginSourceFile(compiler_.getLangOpts(), &compiler_.getPreprocessor());

}
