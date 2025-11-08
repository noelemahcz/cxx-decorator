# cxx-decorator
[ [English](README.md) | 简体中文 ]

[![Language](https://img.shields.io/badge/Language-C%2B%2B23-blue.svg)](https://isocpp.org/)
[![Build](https://img.shields.io/badge/Build-CMake-green.svg)](https://cmake.org/)
[![Dependencies](https://img.shields.io/badge/Dependencies-LLVM%20%26%20Clang-orange.svg)](https://llvm.org/)

`cxx-decorator` 是一个源代码到源代码的转换工具，它通过自定义属性在 C++ 中实现装饰器模式。项目利用 LLVM/Clang LibTooling 基础架构来解析 C++ 的抽象语法树 (AST) 并执行代码转换。

本项目目前处于早期开发阶段。

## 项目动机

在 Python 等语言中，装饰器为修改或增强函数提供了一种便捷方式，可用于日志记录、性能计时或缓存等场景。在 C++ 中，实现这些功能通常需要编写样板代码、使用宏或复杂的模板元编程。本项目旨在为这种模式提供一种更直接、更具声明性的语法。

**之前：**
```cpp
// 手动包装函数
auto timed_func = timing_wrapper(my_function);
```

**使用 `cxx-decorator` 之后：**
```cpp
[[decorator(timing_wrapper)]]
void my_function() {
    // ...
}
```

## 实现原理

`cxx-decorator` 是一个自定义的 Clang 工具，它执行源代码到源代码的转换。

1.  **AST 解析**：像编译器一样，解析 C++ 源文件并构建一个抽象语法树 (AST)。
2.  **属性检测**：遍历 AST，找到被 `[[decorator(SomeClass)]]` 属性注解的函数。
3.  **代码转换**：对于每个被装饰的函数，应用代码转换。当前的实现是重命名原始函数，并注入一个装饰器类的静态实例来包装原始函数。
4.  **代码生成**：该工具输出修改后的 C++ 源代码，可以将结果作为常规构建流程的一部分进行编译。

例如，对于以下输入代码：
```cpp
struct MyDecorator {
  MyDecorator(void (*func)()) {
    // 装饰器逻辑
    func();
  }
};

[[decorator(MyDecorator)]]
void my_function() {
  // 原始函数体
}
```

它将被转换为等效的如下代码：
```cpp
struct MyDecorator {
  MyDecorator(void (*func)()) {
    // 装饰器逻辑
    func();
  }
};

void __decorated_my_function() {
  // 原始函数体
}

static MyDecorator my_function{__decorated_my_function};
```

其核心逻辑通过以下 Clang 组件实现：
-   `ASTFrontendAction`：为源文件设置处理流程。
-   `ASTConsumer`：处理从 AST 中获取的顶层声明。
-   `ParsedAttrInfo`：定义和处理自定义的 `[[decorator]]` 属性。
-   `Rewriter`：管理源代码到源代码的转换。

## 构建工具

### 前置要求
- 兼容 C++23 的编译器 (GCC, Clang)
- CMake (>= 3.20)
- LLVM 和 Clang 开发库 (推荐版本 16 或更高)。请确保 CMake 可以找到它们 (例如，通过设置 `CMAKE_PREFIX_PATH` 或进行全局安装)。

### 使用 Nix

如果您安装了 [Nix](https://nixos.org/download.html) 并启用了 [Flakes](https://nixos.wiki/wiki/Flakes) 功能，您可以直接进入一个已经包含所有必需依赖的开发环境。

```bash
# 进入默认的开发环境
nix develop

# 您也可以选择一个特定的环境 (例如 gcc 或 llvm)
nix develop .#gcc
nix develop .#llvm
```

进入该环境后，您就可以继续执行标准的 CMake 构建步骤。

### 构建步骤
```bash
# 1. 克隆仓库
git clone https://github.com/your-username/cxx-decorator.git
cd cxx-decorator

# 2. 使用 CMake 配置构建
#    如果 CMake 无法自动找到 LLVM，请手动指定其路径：
#    cmake -B build -G Ninja -DCMAKE_PREFIX_PATH=/path/to/llvm
cmake -B build -G Ninja

# 3. 编译 'cxx-decorator' 工具
cmake --build build
```
生成的可执行文件位于 `build/bin/cxx-decorator`。

## 使用方法

对 C++ 源文件运行本工具。您必须通过编译命令数据库 (`compile_commands.json`) 提供编译选项，以确保工具能正确解析代码。

```bash
# 生成 compile_commands.json
cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# 对源文件运行本工具
./build/bin/cxx-decorator /path/to/your_file.cpp
```
转换后的代码将被打印到标准输出。使用 `-o` 标志可将其写入文件：
```bash
./build/bin/cxx-decorator your_file.cpp -o your_file.transformed.cpp
```

## 路线图

-   [ ] **重构装饰器机制**：当前的实现通过函数指针传递被装饰的函数。计划进行重构，探索使用 `template <auto func>` 以实现一个类型更安全、性能更高的编译期机制。
-   [ ] **支持类方法**：扩展功能以支持装饰类成员函数。
-   [ ] **支持装饰器参数**：允许向装饰器传递参数，例如 `[[decorator(cache, "key", 60)]]`。
-   [ ] **构建系统集成**：改进与构建系统（例如通过自定义 CMake 目标）的集成，使源码转换过程无缝化。
-   [ ] **完善测试套件**：添加更多测试，以覆盖复杂的函数签名、模板和边界情况。

## 未来考虑对头文件中函数声明的处理：跳板方案

当前实现依赖于重写函数的声明与定义，这种方式的主要弊端是需要修改头文件，在实际工程中，直接修改头文件会污染源码，而如果选择生成副本，则会显著增加构建系统的集成复杂度，并且头文件存在包含顺序和相对路径问题，很容易导致 ODR 违例。

为了解决这个重大阻碍，未来的架构预期将转向一种**“跳板函数” (Trampoline Function)** 方案，其核心目的就是避免对头文件的任何修改（或生成副本）。

### 工作原理

该方案将函数的声明与定义解耦，仅在 `.cpp` 实现文件中进行代码转换。

**源代码:**
```cpp
// foo.h
void foo(int x);

// foo.cpp
#include "foo.h"
[[decorator(Timer)]]
void foo(int x) { /* ... 函数体 ... */ }
```

**转换后的 `foo.cpp`:**
```cpp
// foo.cpp (转换后)
#include "foo.h"

// 用户定义的装饰器模板
template <auto Func> struct Timer { /* ... */ };

// 1. 重命名原始函数
void __decorated_foo(int x) { /* ... */ }

// 2. 在文件作用域创建静态装饰器实例，
//    确保了实例在程序启动时构造，以支持注册模式及有状态的装饰器。
static Timer<__decorated_foo> foo_decorator_instance;

// 3. 生成一个同名的“跳板函数”，作为 foo 唯一的新定义。
void foo(int x) {
    foo_decorator_instance(x);
}
```

### 潜在挑战

1. 无法处理在头文件中定义的的函数，典型如函数模板和内联函数，不过这个问题不影响大部分场景，因此未来再进行考虑
2. 跳板函数可能导致多一层调用开销，但应该可以通过开启 LTO 来避免。
