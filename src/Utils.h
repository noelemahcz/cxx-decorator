#pragma once


#include <llvm/Support/raw_ostream.h>

#include <format>


#define PRINT(...) llvm::errs() << std::format(__VA_ARGS__)
#define PRINTLN(...) llvm::errs() << std::format(__VA_ARGS__) << '\n'
