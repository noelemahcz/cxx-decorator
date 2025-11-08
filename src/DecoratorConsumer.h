#pragma once


#include <clang/AST/ASTConsumer.h>
#include <clang/Basic/SourceLocation.h>
#include <llvm/ADT/SmallVector.h>


namespace clang {
class Rewriter;
} // namespace clang


struct PendingDecoration {
  clang::FunctionDecl* func;
  clang::CXXRecordDecl* deco;
  clang::SourceRange attr_range;
};

using PendingDecorations = llvm::SmallVector<PendingDecoration>;


class DecoratorConsumer final : public clang::ASTConsumer {
  friend class ParsedDecoratorAttrInfo;

public:
  explicit DecoratorConsumer(clang::ASTContext& ctx, clang::Rewriter& rewriter)
      : ctx_{ctx}, rewriter_{rewriter} {}

private:
  auto HandleTranslationUnit(clang::ASTContext& ctx) -> void override;
  auto HandleTopLevelDecl(clang::DeclGroupRef group) -> bool override;

private:
  clang::ASTContext& ctx_;
  clang::Rewriter& rewriter_;
  PendingDecorations pending_decorations_;
};
