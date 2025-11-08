#include "DecoratorAttribute.h"

#include "DecoratorConsumer.h"
#include "Utils.h"

#include <clang/Basic/DiagnosticSema.h>
#include <clang/Sema/Lookup.h>
#include <clang/Sema/ParsedAttr.h>
#include <clang/Sema/Sema.h>


using clang::AttributeDeclKind;
using clang::CXXRecordDecl;
using clang::Decl;
using clang::DiagnosticsEngine;
using clang::FunctionDecl;
using clang::IdentifierLoc;
using clang::ParsedAttr;
using clang::ParsedAttrInfoRegistry;
using clang::Sema;
using clang::Stmt;


// LLVM static registration pattern
static ParsedAttrInfoRegistry::Add<ParsedDecoratorAttrInfo> attr("decorator",
                                                                 "");


ParsedDecoratorAttrInfo::ParsedDecoratorAttrInfo() {
  PRINTLN("DecoratorAttrInfo::DecoratorAttrInfo");

  // NOLINTBEGIN(*-avoid-c-arrays)
  static constexpr Spelling kSpellings[] = {
      {ParsedAttr::AS_CXX11, "decorator"}};
  // NOLINTEND(*-avoid-c-arrays)
  Spellings = kSpellings;
  NumArgs = 1;
  OptArgs = 0;
}


auto ParsedDecoratorAttrInfo::handleDeclAttribute(Sema& sema, Decl* decl,
                                                  ParsedAttr const& attr) const
    -> AttrHandling {
  PRINTLN("DecoratorAttrInfo::handleDeclAttribute");

  auto* const func = dyn_cast<FunctionDecl>(decl);
  if (func == nullptr) {
    auto const id = sema.getDiagnostics().getCustomDiagID(
        DiagnosticsEngine::Error,
        "'decorator' attribute only allowed on functions");
    sema.Diag(attr.getLoc(), id);
    return AttributeNotApplied;
  }

  if (attr.getNumArgs() != 1) {
    auto const id = sema.getDiagnostics().getCustomDiagID(
        DiagnosticsEngine::Error,
        "'decorator' attribute only accepts exactly one argument");
    sema.Diag(attr.getLoc(), id);
    return AttributeNotApplied;
  }

  auto* const arg = cast<IdentifierLoc*>(attr.getArg(0));
  if (arg == nullptr) {
    auto const id = sema.getDiagnostics().getCustomDiagID(
        DiagnosticsEngine::Error, "'decorator' arg type error");
    sema.Diag(attr.getLoc(), id);
    return AttributeNotApplied;
  }

  clang::LookupResult result{sema, arg->getIdentifierInfo(), arg->getLoc(),
                             Sema::LookupNameKind::LookupOrdinaryName};
  sema.LookupName(result, sema.getCurScope());

  if (!result.isSingleResult()) {
    auto const id = sema.getDiagnostics().getCustomDiagID(
        DiagnosticsEngine::Error, "'decorator' with wrong arg");
    sema.Diag(attr.getLoc(), id);

    // This is handled automatically by `LookupResult`'s destructor.
    // if (result.isAmbiguous()) {
    //   sema.Diag(deco_loc, diag::err_ambiguous_reference)
    //       << result.getLookupName() << result.getContextRange();
    //   for (auto* const d : result) {
    //     sema.Diag(d->getLocation(), diag::note_ambiguous_candidate) <<
    //     d;
    //   }
    // }

    return AttributeNotApplied;
  }

  auto* const deco = result.getAsSingle<CXXRecordDecl>();
  if (deco == nullptr) {
    auto const id = sema.getDiagnostics().getCustomDiagID(
        DiagnosticsEngine::Error, "'decorator' with wrong arg type");
    sema.Diag(arg->getLoc(), id);
    return AttributeNotApplied;
  }

  // Out-of-band transmission
  auto& consumer = sema.getASTConsumer();
  assert(dynamic_cast<DecoratorConsumer*>(&consumer) != nullptr &&
         "Type assertion failed: `Sema::getASTConsumer() is not a "
         "`MyConsumer` object!");

  // NOLINTNEXTLINE(*-static-cast-downcast)
  auto& my_consumer = static_cast<DecoratorConsumer&>(consumer);
  my_consumer.pending_decorations_.emplace_back(func, deco, attr.getRange());

  // `AnnotateAttr` can only carry `Expr*`, not `Decl*`.
  // decl->addAttr(AnnotateAttr::Create(
  //       sema.Context, "decorator", &deco, 1, attr.getRange()));

  return AttributeApplied;
}


static auto DiagWrongType(Sema& sema, ParsedAttr const& attr) -> void {
  sema.Diag(attr.getLoc(), clang::diag::warn_attribute_wrong_decl_type)
      << attr << attr.isRegularKeywordAttribute()
      << AttributeDeclKind::ExpectedFunction;
}


auto ParsedDecoratorAttrInfo::diagAppertainsToDecl(Sema& sema,
                                                   ParsedAttr const& attr,
                                                   Decl const* decl) const
    -> bool {
  PRINTLN("DecoratorAttrInfo::diagAppertainsToDecl");

  if (!isa<FunctionDecl>(decl)) {
    DiagWrongType(sema, attr);
    return false;
  }
  return true;
}


auto ParsedDecoratorAttrInfo::diagAppertainsToStmt(Sema& sema,
                                                   ParsedAttr const& attr,
                                                   Stmt const* /*stmt*/) const
    -> bool {
  PRINTLN("DecoratorAttrInfo::diagAppertainsToStmt");

  DiagWrongType(sema, attr);
  return false;
}
