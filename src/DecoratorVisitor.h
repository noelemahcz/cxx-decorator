#pragma once


// class MyVisitor final : public RecursiveASTVisitor<MyVisitor> {
//   using Base = RecursiveASTVisitor<MyVisitor>;
//
// public:
//   explicit MyVisitor(ASTContext& ctx) : ctx_{ctx} {}
//
//   auto TraverseFunctionDecl(FunctionDecl* decl) -> bool {
//     auto const name = decl->getNameAsString();
//     llvm::outs() << "TraverseFunctionDecl() " << name << "\n";
//     if (name == "foo") {
//       for (auto* const attr : decl->attrs()) {
//         llvm::outs() << attr->getKind() << '\n';
//         llvm::outs() << attr->getSpelling() << '\n';
//       }
//       decl->dump();
//       static_cast<Base*>(this)->TraverseFunctionDecl(decl);
//       decl->dump();
//     }
//     return true;
//   }
//
//   auto VisitFunctionDecl(FunctionDecl* decl) -> bool {
//     PRINTLN("==================================================");
//     PRINTLN("VisitFunctionDecl: {}[{}]", decl->getNameAsString(),
//             static_cast<void*>(decl));
//
//     PRINTLN("CanonicalDecl: {}",
//     static_cast<void*>(decl->getCanonicalDecl()));
//     PRINTLN("IsThisADefinition: {}", decl->isThisDeclarationADefinition());
//     PRINTLN("IsDefined: {}", decl->isDefined());
//     PRINTLN("Definition: {}", static_cast<void*>(decl->getDefinition()));
//
//     decl->dump();
//
//     return true;
//   }
//
//   auto VisitFunctionDecl(FunctionDecl* decl) -> bool {
//     PRINTLN("==================================================");
//     PRINTLN("VisitFunctionDecl: {}", decl->getNameAsString());
//
//     auto const is_specialization = decl->isFunctionTemplateSpecialization();
//     PRINTLN("isFunctionTemplateSpecialization: {}", is_specialization);
//
//     auto const templated_kind = decl->getTemplatedKind();
//     PRINTLN("TemplatedKind: {}", static_cast<int>(templated_kind));
//
//     auto const specialization_kind = decl->getTemplateSpecializationKind();
//     PRINTLN("SpecializationKind: {}", static_cast<int>(specialization_kind));
//
//     auto* const specialization_info = decl->getTemplateSpecializationInfo();
//     PRINTLN("specialization_info: {}",
//     static_cast<void*>(specialization_info));
//
//     // decl->dump();
//
//     if (templated_kind != clang::FunctionDecl::TK_NonTemplate) {
//       for (auto* const spec :
//            decl->getDescribedFunctionTemplate()->specializations()) {
//         PRINTLN("spec:");
//         spec->dump();
//       }
//     }
//
//     return true;
//   }
//
//   auto VisitReturnStmt(ReturnStmt* stmt) -> bool {
//     llvm::outs() << "VisitReturnStmt()\n";
//     llvm::StringRef const str = "world";
//     auto const str_lit_t =
//         ctx_.getStringLiteralArrayType(ctx_.CharTy, str.size());
//     auto* const str_lit = StringLiteral::Create(
//         ctx_, str, StringLiteralKind::Ordinary, false, str_lit_t, {});
//     stmt->setRetValue(str_lit);
//     return true;
//   }
//
// private:
//   ASTContext& ctx_;
// };
