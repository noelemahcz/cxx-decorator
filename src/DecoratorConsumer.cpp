#include "DecoratorConsumer.h"

#include "Utils.h"

#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclCXX.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/DiagnosticSema.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/Lexer.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/raw_ostream.h>

#include <format>
#include <ranges>
#include <string_view>


using clang::ASTContext;
using clang::CXXRecordDecl;
using clang::DeclGroupRef;
using clang::DiagnosticsEngine;
using clang::FunctionDecl;
using clang::Lexer;
using clang::Rewriter;
using clang::SourceRange;
using clang::tok::TokenKind;
using llvm::SmallDenseMap;
using llvm::SmallString;
using llvm::SmallVector;
using llvm::StringRef;
using llvm::raw_svector_ostream;


static constexpr auto kOriginalPrefix = "__original_";
static constexpr auto kDecoratedPrefix = "__decorated_";


auto DecoratorConsumer::HandleTopLevelDecl(DeclGroupRef /*group*/) -> bool {
  PRINTLN("DecoratorConsumer::HandleTopLevelDecl");

  return true;
}


static auto RemoveDecoAttrDecls(PendingDecorations const& func_deco_info,
                                Rewriter& rewriter) -> bool {
  PRINTLN("RemoveDecoAttrDecls");
}


static auto RenameOriginalDecls(PendingDecorations const& decorations,
                                Rewriter& rewriter) -> void {
  PRINTLN("RenameOriginalDecls");

  for (auto const& d : decorations) {
    auto const name_range = d.func->getNameInfo().getSourceRange();
    auto const new_name =
        std::format("{}{}", kOriginalPrefix, d.func->getName().data());
    // https://clang.llvm.org/doxygen/classclang_1_1Rewriter.html#a5fd6f665d719a8f2dbd6a6e6b5e1436b
    // This method returns true (and does nothing) if the
    // input location was not rewritable, false otherwise.
    auto const res = rewriter.ReplaceText(name_range, new_name);
    assert(!res);
  }
}


template <unsigned InternalLen>
static auto GenerateThunkDefinition(FunctionDecl* def,
                                    SmallString<InternalLen>& thunk) -> void {
  PRINTLN("GenerateThunkDefinition");

  raw_svector_ostream os{thunk};

  auto const& sm = def->getASTContext().getSourceManager();
  auto const& lang = def->getASTContext().getLangOpts();

  auto const decl_range = def->clang::DeclaratorDecl::getSourceRange();
  auto const end_of_end =
      Lexer::getLocForEndOfToken(decl_range.getEnd(), 0, sm, lang);
  auto const decl_str =
      std::string_view(sm.getCharacterData(decl_range.getBegin()),
                       sm.getCharacterData(end_of_end));

  os << decl_str << " {\n  return " << kDecoratedPrefix << def->getName()
     << '(';

  auto params_str =
      def->parameters() |
      std::views::transform([](clang::ParmVarDecl* param) -> StringRef {
        return param->getName();
      }) |
      std::views::join_with(',');

  std::ranges::copy(params_str, std::back_inserter(thunk));

  os << ");" << "\n}";
}


auto DecoratorConsumer::HandleTranslationUnit(ASTContext& ctx) -> void {
  PRINTLN("DecoratorConsumer::HandleTranslationUnit");

  PRINTLN("pending_decorations");
  for (auto const& [func, deco, attr_range] : pending_decorations_) {
    PRINTLN("Function {}: {}", func->getName().data(),
            static_cast<void*>(func));
    PRINTLN("Decorator {}: {}", deco->getName().data(),
            static_cast<void*>(deco));
    attr_range.dump(ctx_.getSourceManager());
  }

  RenameOriginalDecls(pending_decorations_, rewriter_);

  auto const& sm = ctx.getSourceManager();
  auto const& lang = ctx.getLangOpts();

  struct DecoratorInfo {
    CXXRecordDecl* deco;
    SmallVector<SourceRange, 2> attr_ranges;
  };

  SmallDenseMap<FunctionDecl*, DecoratorInfo, 8> cano_deco_map;
  for (auto const& [func, deco, attr_range] : pending_decorations_) {
    auto* const canonical = func->getCanonicalDecl();

    DecoratorInfo& info = cano_deco_map[canonical];
    if (info.deco == nullptr) {
      info.deco = deco;

    } else if (info.deco != deco) {
      auto& diags = ctx.getDiagnostics();
      auto const id = diags.getCustomDiagID(
          DiagnosticsEngine::Error,
          "conflicting 'decorator' attributes on function %0");
      diags.Report(attr_range.getBegin(), id) << func->getDeclName();
      for (auto const& prev_attr_range : info.attr_ranges) {
        diags.Report(prev_attr_range.getBegin(),
                     clang::diag::note_previous_attribute);
      }
      return;
    }

    info.attr_ranges.push_back(attr_range);
  }

  PRINTLN("cano_deco_map");
  for (auto const& [cano, deco_info] : cano_deco_map) {
    auto const& [deco, attr_ranges] = deco_info;
    PRINTLN("Canonical {}: {}", cano->getName().data(),
            static_cast<void*>(cano));
    PRINTLN("Decorator {}: {}", deco->getName().data(),
            static_cast<void*>(deco));
    for (auto const& attr_range : attr_ranges) {
      attr_range.dump(ctx_.getSourceManager());
    }
  }

  for (auto const& [cano, deco_info] : cano_deco_map) {

    auto const& [deco, attr_ranges] = deco_info;
    for (auto range : attr_ranges) {

      auto const next_tok =
          Lexer::findNextToken(range.getEnd(), sm, lang, false);
      if (next_tok && next_tok->is(TokenKind::comma)) {
        range.setEnd(next_tok->getLocation());
        // range.setEnd( Lexer::getLocForEndOfToken(
        //     next_tok->getLocation(), 0, sm, lang));

      } else {
        auto const prev_tok =
            Lexer::findPreviousToken(range.getBegin(), sm, lang, false);
        if (prev_tok && prev_tok->is(TokenKind::comma)) {
          range.setBegin(prev_tok->getLocation());

        } else if (prev_tok && next_tok && prev_tok->is(TokenKind::l_square) &&
                   next_tok->is(TokenKind::r_square)) {

          auto const prev_prev_tok = Lexer::findPreviousToken(
              prev_tok->getLocation(), sm, lang, false);
          auto const next_next_tok =
              Lexer::findNextToken(next_tok->getLocation(), sm, lang, false);
          if (prev_prev_tok && next_next_tok &&
              prev_prev_tok->is(TokenKind::l_square) &&
              next_next_tok->is(TokenKind::r_square)) {

            range.setBegin(prev_prev_tok->getLocation());
            range.setEnd(next_next_tok->getLocation());
            // range.setEnd(Lexer::getLocForEndOfToken(
            //     next_next_tok->getLocation(), 0, sm, lang));
          }
        }
      }

      rewriter_.RemoveText(range);
    }

    PRINTLN("Check IsDefined For: {}", cano->getName().data());

    if (!cano->isDefined()) {
      continue;
    }

    PRINTLN("IsDefined: true");

    auto* const def = cano->getDefinition();
    assert(def != nullptr);

    PRINTLN("Definition: {}", static_cast<void const*>(def));

    auto const end_of_def = def->getEndLoc();
    if (!sm.isInMainFile(end_of_def)) {
      continue;
    }

    PRINTLN("IsInMainFile: true");

    PRINT("EndOfDef: ");
    end_of_def.dump(sm);

    // auto const insert_loc = Lexer::findLocationAfterToken(
    //     def->getEndLoc(), TokenKind::r_brace, sm, lang, true);
    auto const insert_loc = Lexer::getLocForEndOfToken(end_of_def, 0, sm, lang);

    PRINT("InsertLoc: ");
    insert_loc.dump(sm);

    PRINT("InsertLoc Next: ");
    insert_loc.getLocWithOffset(1).dump(sm);

    // Generate decorated functor & function wrapper.

    SmallString<32> thunk;
    GenerateThunkDefinition(def, thunk);

    // static Decorator __decorated_func{__original_func};
    // int func(bool b, double d, string s) { return __decorated_func(b,d,s); }
    auto const generated_src =
        std::format("\n\nstatic {2} {0}{3} {{{1}{3}}};\n\n{4}",
                    kDecoratedPrefix, kOriginalPrefix, deco->getName().data(),
                    cano->getName().data(), thunk.c_str());

    rewriter_.InsertTextAfterToken(end_of_def, generated_src);
  }
}
