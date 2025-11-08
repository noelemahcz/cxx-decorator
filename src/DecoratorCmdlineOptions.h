#pragma once


#include <llvm/Support/CommandLine.h>

#include <string>


extern llvm::cl::OptionCategory decorator_category;
extern llvm::cl::opt<std::string> output_file;
