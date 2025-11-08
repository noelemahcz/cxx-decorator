#pragma once


#include <clang/Frontend/FrontendAction.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <llvm/ADT/StringRef.h>

#include <memory>


class DecoratorAction final : public clang::ASTFrontendAction {
private:
  auto CreateASTConsumer(clang::CompilerInstance& compiler,
                         llvm::StringRef file)
      -> std::unique_ptr<clang::ASTConsumer> override;

  auto EndSourceFileAction() -> void override;

private:
  clang::Rewriter rewriter_;
};
