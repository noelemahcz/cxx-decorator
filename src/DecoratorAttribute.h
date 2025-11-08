#pragma once


#include <clang/Basic/ParsedAttrInfo.h>


class ParsedDecoratorAttrInfo final : public clang::ParsedAttrInfo {
public:
  ParsedDecoratorAttrInfo();

  auto diagAppertainsToDecl(clang::Sema& sema, clang::ParsedAttr const& attr,
                            clang::Decl const* decl) const -> bool override;

  auto diagAppertainsToStmt(clang::Sema& sema, clang::ParsedAttr const& attr,
                            clang::Stmt const* stmt) const -> bool override;

  auto handleDeclAttribute(clang::Sema& sema, clang::Decl* decl,
                           clang::ParsedAttr const& attr) const
      -> AttrHandling override;
};
