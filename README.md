# cxx-decorator
[ English | [简体中文](README.zh.md) ]

[![Language](https://img.shields.io/badge/Language-C%2B%2B23-blue.svg)](https://isocpp.org/)
[![Build](https://img.shields.io/badge/Build-CMake-green.svg)](https://cmake.org/)
[![Dependencies](https://img.shields.io/badge/Dependencies-LLVM%20%26%20Clang-orange.svg)](https://llvm.org/)

`cxx-decorator` is a source-to-source transformation tool that implements a decorator pattern in C++ using a custom attribute. It uses the LLVM/Clang LibTooling infrastructure to parse the C++ Abstract Syntax Tree (AST) and perform code transformations.

This project is currently in an early stage of development.

## Motivation

In languages like Python, decorators provide a way to modify or enhance functions and methods for tasks like logging, timing, or caching. In C++, this often requires boilerplate code, macros, or complex template metaprogramming. This project aims to provide a more direct, declarative syntax for this pattern.

**Before:**
```cpp
// Manually wrapping a function
auto timed_func = timing_wrapper(my_function);
```

**With `cxx-decorator`:**
```cpp
[[decorator(timing_wrapper)]]
void my_function() {
    // ...
}
```

## How It Works

`cxx-decorator` is a custom Clang tool that performs a source-to-source transformation.

1.  **AST Parsing**: It parses C++ source files and builds an AST, just like a compiler.
2.  **Attribute Detection**: It traverses the AST to find functions annotated with the `[[decorator(SomeClass)]]` attribute.
3.  **Code Transformation**: For each decorated function, it applies a code transformation. The current implementation renames the original function and injects a static instance of the decorator class, which wraps the original function.
4.  **Source Generation**: The tool outputs the modified C++ source code, which can then be compiled as part of a normal build process.

For example, this input code:
```cpp
struct MyDecorator {
  MyDecorator(void (*func)()) {
    // Decorator logic
    func();
  }
};

[[decorator(MyDecorator)]]
void my_function() {
  // Original function body
}
```

Is transformed into the following equivalent code:
```cpp
struct MyDecorator {
  MyDecorator(void (*func)()) {
    // Decorator logic
    func();
  }
};

void __decorated_my_function() {
  // Original function body
}

static MyDecorator my_function{__decorated_my_function};
```

The core logic is implemented using the following Clang components:
-   `ASTFrontendAction`: To set up the processing for a source file.
-   `ASTConsumer`: To handle top-level declarations from the AST.
-   `ParsedAttrInfo`: To define and handle the custom `[[decorator]]` attribute.
-   `Rewriter`: To manage the source-to-source code transformations.

## Building the Tool

### Prerequisites
- A C++23 compatible compiler (GCC, Clang)
- CMake (>= 3.20)
- LLVM and Clang development libraries (version 16 or newer recommended). Ensure they are discoverable by CMake (e.g., by setting `CMAKE_PREFIX_PATH` or using system-wide installation).

### Using Nix

If you have [Nix](https://nixos.org/download.html) with [Flakes](https://nixos.wiki/wiki/Flakes) enabled, you can enter a development shell with all the required dependencies pre-installed.

```bash
# Enter the default development shell
nix develop

# You can also select a specific shell (e.g., gcc or llvm)
nix develop .#gcc
nix develop .#llvm
```

Once inside the shell, you can proceed with the normal CMake build steps.

### Build Steps
```bash
# 1. Clone the repository
git clone https://github.com/your-username/cxx-decorator.git
cd cxx-decorator

# 2. Configure the build using CMake
#    If LLVM is not found automatically, specify its location:
#    cmake -B build -G Ninja -DCMAKE_PREFIX_PATH=/path/to/llvm
cmake -B build -G Ninja

# 3. Compile the 'cxx-decorator' tool
cmake --build build
```
The executable will be located at `build/bin/cxx-decorator`.

## Usage

Run the tool on a C++ source file. You must provide the compilation commands via a compile commands database (`compile_commands.json`) for the tool to correctly parse the code.

```bash
# Generate compile_commands.json
cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Run the tool on a source file
./build/bin/cxx-decorator /path/to/your_file.cpp
```
The transformed code will be printed to standard output. Use the `-o` flag to write to a file:
```bash
./build/bin/cxx-decorator your_file.cpp -o your_file.transformed.cpp
```

## Roadmap

-   [ ] **Refactor Decorator Mechanism**: The current implementation passes the decorated function as a function pointer. A planned refactoring will explore using `template <auto func>` for a more type-safe and performant compile-time mechanism.
-   [ ] **Support for Class Methods**: Extend functionality to decorate class member functions.
-   [ ] **Decorator Arguments**: Allow passing arguments to decorators, e.g., `[[decorator(cache, "key", 60)]]`.
-   [ ] **Build System Integration**: Improve integration with build systems (e.g., via custom CMake targets) to make the source-to-source transformation seamless.
-   [ ] **Comprehensive Test Suite**: Add more tests to cover complex function signatures, templates, and edge cases.

## Future Consideration: Handling Function Declarations in Headers

The current implementation relies on rewriting function declarations and definitions. Its major drawback is the need to modify header files. In real-world projects, modifying headers directly pollutes the source tree. Alternatively, generating copies of headers significantly complicates the build system integration, and can easily lead to ODR violations due to issues with include order and relative paths.

To overcome this significant obstacle, the future architecture is expected to shift to a **"Trampoline Function"** approach, with the core goal of avoiding any modification (or generation) of header files.

### How It Works

This approach decouples a function's declaration from its definition, applying code transformations only within the `.cpp` implementation file.

**Source Code:**
```cpp
// foo.h
void foo(int x);

// foo.cpp
#include "foo.h"
[[decorator(Timer)]]
void foo(int x) { /* ... body ... */ }
```

**Transformed `foo.cpp`:**
```cpp
// foo.cpp (transformed)
#include "foo.h"

// User-defined decorator template
template <auto Func> struct Timer { /* ... */ };

// 1. Rename the original function
void __decorated_foo(int x) { /* ... */ }

// 2. A static decorator instance is created at file scope, ensuring it is
//    constructed at program startup to support registration patterns and
//    stateful decorators.
static Timer<__decorated_foo> foo_decorator_instance;

// 3. A "trampoline" function with the same name is generated as the new, single definition for `foo`.
void foo(int x) {
    foo_decorator_instance(x);
}
```

### Potential Challenges

1.  It cannot handle functions defined in header files, typically function templates and inline functions. However, this does not affect most scenarios and will be considered in the future.
2.  The trampoline function may cause an extra layer of call overhead, but this can likely be avoided by enabling Link-Time Optimization (LTO).

## Contributing
Contributions are welcome. Please open an issue to discuss your ideas before submitting a pull request.
