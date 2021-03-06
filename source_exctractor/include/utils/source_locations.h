//=============================================================================
//               	Clomp: A Clang-based OpenMP Frontend
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//=============================================================================

#include <string>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <sstream>

#include <llvm/Support/raw_ostream.h>

namespace clang {
class SourceLocation;
class SourceRange;
class SourceManager;
}

namespace utils {

std::string FileName(clang::SourceLocation const& l, clang::SourceManager const& sm);

std::string FileId(clang::SourceLocation const& l, clang::SourceManager const& sm);

unsigned Line(clang::SourceLocation const& l, clang::SourceManager const& sm);

std::pair<unsigned, unsigned> Line(clang::SourceRange const& r, clang::SourceManager const& sm);

unsigned Column(clang::SourceLocation const& l, clang::SourceManager const& sm);

std::pair<unsigned, unsigned> Column(clang::SourceRange const& r, clang::SourceManager const& sm);

std::string location(clang::SourceLocation const& l, clang::SourceManager const& sm);

} // End utils namespace

