#include "DecoratorAction.h"

#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>


namespace cl = llvm::cl;

using clang::tooling::ClangTool;
using clang::tooling::CommonOptionsParser;
using clang::tooling::newFrontendActionFactory;


cl::OptionCategory decorator_category{"cxx-decorator options"};
cl::opt<std::string> output_file{"o", cl::cat{decorator_category},
                                 cl::desc{"Write output to <file>"},
                                 cl::value_desc{"file"}};

static cl::extrahelp common_help{CommonOptionsParser::HelpMessage};


auto main(int argc, char const** argv) -> int {
  auto expected_parser =
      CommonOptionsParser::create(argc, argv, decorator_category);
  if (!expected_parser) {
    llvm::errs() << expected_parser.takeError();
    return 1;
  }

  auto& parser = expected_parser.get();
  ClangTool tool{parser.getCompilations(), parser.getSourcePathList()};

  return tool.run(newFrontendActionFactory<DecoratorAction>().get());
}
