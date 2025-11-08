add_executable(cxx-decorator
  DecoratorAction.cpp
  DecoratorAttribute.cpp
  DecoratorConsumer.cpp
  DecoratorVisitor.cpp
  Main.cpp)

# target_precompile_headers(cxx-decorator PRIVATE pch.h)

target_compile_options(cxx-decorator PRIVATE -Wall -Wextra)
# target_compile_options(cxx-decorator PRIVATE -stdlib=libc++)

# LLVM
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
target_compile_definitions(cxx-decorator PRIVATE ${LLVM_DEFINITIONS_LIST})

target_include_directories(cxx-decorator PRIVATE ${LLVM_INCLUDE_DIRS})

llvm_map_components_to_libnames(llvm_libs support)
target_link_libraries(cxx-decorator PRIVATE ${llvm_libs} clang-cpp)
