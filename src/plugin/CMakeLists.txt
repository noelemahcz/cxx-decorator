set(target ${PROJECT_NAME}-plugin)

add_library(${target} SHARED main.cpp)

target_compile_options(${target} PRIVATE -Wall -Wextra)

# LLVM
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
target_compile_definitions(${target} PRIVATE ${LLVM_DEFINITIONS_LIST})

target_include_directories(${target} PRIVATE ${LLVM_INCLUDE_DIRS})

llvm_map_components_to_libnames(llvm_libs support)
target_link_libraries(${target} PRIVATE ${llvm_libs} clang-cpp)
