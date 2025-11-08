set(input_source ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp)
set(output_source ${CMAKE_CURRENT_BINARY_DIR}/generated/test.cpp)

add_custom_command(
  OUTPUT ${output_source}

  COMMAND
    cxx-decorator
    -p ${CMAKE_BINARY_DIR}
    -o ${output_source}
    ${input_source}

  MAIN_DEPENDENCY
    ${input_source}

  DEPENDS
    ${input_source}

  VERBATIM
  USES_TERMINAL
)

add_executable(test ${output_source})
