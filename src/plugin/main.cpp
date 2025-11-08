#include <clang/AST/ASTConsumer.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Frontend/FrontendPluginRegistry.h>
#include <clang/Sema/ParsedAttr.h>
#include <clang/Sema/Sema.h>


using namespace clang;


namespace {


class MyVisitor final : public RecursiveASTVisitor<MyVisitor> {
  using Base = RecursiveASTVisitor<MyVisitor>;

public:
  explicit MyVisitor(ASTContext& ctx) : ctx_{ctx} {}

  auto TraverseFunctionDecl(FunctionDecl* decl) -> bool {
    llvm::outs() << "TraverseFunctionDecl()\n";
    if (decl->getName() == "foo") {
      decl->dump();
      static_cast<Base*>(this)->TraverseFunctionDecl(decl);
      decl->dump();
    }
    return true;
  }

  auto VisitReturnStmt(ReturnStmt* stmt) -> bool {
    llvm::StringRef const str = "world";
    auto const str_lit_t =
        ctx_.getStringLiteralArrayType(ctx_.CharTy, str.size());
    auto* const str_lit_e = StringLiteral::Create(
        ctx_, str, StringLiteralKind::Ordinary, false, str_lit_t, {});

    auto const char_const_ptr_t =
        ctx_.getPointerType(ctx_.getConstType(ctx_.CharTy));
    auto* const implicit_arr_to_ptr_e = ImplicitCastExpr::Create(
        ctx_, char_const_ptr_t, CastKind::CK_ArrayToPointerDecay, str_lit_e,
        nullptr, ExprValueKind::VK_PRValue, FPOptionsOverride{});

    stmt->setRetValue(implicit_arr_to_ptr_e);
    return true;
  }


private:
  ASTContext& ctx_;
};


class MyConsumer final : public ASTConsumer {
public:
  explicit MyConsumer(ASTContext& ctx) : visitor_{ctx} {}

private:
  // auto HandleTranslationUnit(clang::ASTContext& ctx) -> void override {
  //   llvm::outs() << "HandleTranslationUnit()\n";
  //   visitor_.TraverseDecl(ctx.getTranslationUnitDecl());
  // }

  auto HandleTopLevelDecl(DeclGroupRef group) -> bool override {
    llvm::outs() << "HandleTopLevelDecl()\n";
    for (auto* const decl : group) {
      visitor_.TraverseDecl(decl);
    }
    // for (auto* const decl : group) {
    //   decl->dump();
    //   visitor_.TraverseDecl(decl);
    //   // if (auto* const func_decl = dyn_cast<FunctionDecl>(decl)) {
    //   //   func_decl->dump();
    //   // }
    // }
    return true;
  }

private:
  MyVisitor visitor_;
};


class MyAction final : public PluginASTAction {
  auto CreateASTConsumer(CompilerInstance& compiler, StringRef /*in_file*/)
      -> std::unique_ptr<ASTConsumer> override {
    llvm::outs() << "CreateASTConsumer()\n";
    return std::make_unique<MyConsumer>(compiler.getASTContext());
  }

  auto ParseArgs(CompilerInstance const& /*compiler*/,
                 std::vector<std::string> const& /*args*/) -> bool override {
    llvm::outs() << "ParseArgs()\n";
    return true;
  }

  auto getActionType() -> ActionType override {
    llvm::outs() << "getActionType()\n";
    return ActionType::AddBeforeMainAction;
  }
};


FrontendPluginRegistry::Add<MyAction> X("clang-ast-plugin", "clang ast plugin");


class DecoratorAttrInfo final : public ParsedAttrInfo {
public:
  DecoratorAttrInfo() {
    // NOLINTBEGIN(*-avoid-c-arrays, modernize-use-designated-initializers)
    static constexpr Spelling kSpellings[] = {
        {ParsedAttr::AS_CXX11, "decorator"}};
    // NOLINTEND(*-avoid-c-arrays, modernize-use-designated-initializers)
    Spellings = kSpellings;
    OptArgs = 1;
  }

  auto diagAppertainsToDecl(Sema& sema, ParsedAttr const& attr,
                            Decl const* decl) const -> bool override {
    if (!isa<FunctionDecl>(decl)) {
      diag(sema, attr);
      return false;
    }
    return true;
  }

  auto diagAppertainsToStmt(Sema& sema, ParsedAttr const& attr,
                            [[maybe_unused]] Stmt const* stmt) const
      -> bool override {
    diag(sema, attr);
    return false;
  }

  auto handleDeclAttribute(Sema& sema, Decl* decl, ParsedAttr const& attr) const
      -> AttrHandling override {
    if (!decl->getDeclContext()->isFileContext()) {
      unsigned id = sema.getDiagnostics().getCustomDiagID(
          DiagnosticsEngine::Error,
          "'decorator' attribute only allowed at file scope");
      sema.Diag(attr.getLoc(), id);
      return AttributeNotApplied;
    }
    return AttributeApplied;
  }

private:
  static auto diag(Sema& sema, ParsedAttr const& attr) -> void {
    sema.Diag(attr.getLoc(), diag::warn_attribute_wrong_decl_type)
        << attr << attr.isRegularKeywordAttribute()
        << AttributeDeclKind::ExpectedFunction;
  }
};


// ParsedAttrInfoRegistry::Add<DecoratorAttrInfo> attr("decorator", "");


} // namespace
