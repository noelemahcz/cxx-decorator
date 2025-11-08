#include "DecoratorAction.h"

#include "DecoratorCmdlineOptions.h"
#include "DecoratorConsumer.h"
#include "scope_guard.hpp"

#include <llvm/Support/raw_ostream.h>

#include <system_error>


using clang::ASTConsumer;
using clang::CompilerInstance;
using llvm::StringRef;


auto DecoratorAction::CreateASTConsumer(CompilerInstance& compiler,
                                        StringRef /*in_file*/)
    -> std::unique_ptr<ASTConsumer> {
  rewriter_.setSourceMgr(compiler.getSourceManager(), compiler.getLangOpts());
  return std::make_unique<DecoratorConsumer>(compiler.getASTContext(),
                                             rewriter_);
}


auto DecoratorAction::EndSourceFileAction() -> void {
  auto const guard = sg::make_scope_guard(
      [this]() -> void { ASTFrontendAction::EndSourceFileAction(); });

  auto& sm = getCompilerInstance().getSourceManager();

  if (output_file.getNumOccurrences() < 1) {
    rewriter_.getEditBuffer(sm.getMainFileID()).write(llvm::outs());

  } else {
    auto ec = std::error_code{};
    auto file = llvm::raw_fd_ostream{output_file, ec};
    if (ec) {
      llvm::errs() << "Failed to open output file: " << ec.message() << "\n";
      return;
    }
    rewriter_.getEditBuffer(sm.getMainFileID()).write(file);
  }
}
