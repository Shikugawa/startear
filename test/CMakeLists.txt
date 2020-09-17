cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

project(startear_test CXX)

# the test program
add_executable(tokenizer_test test.cpp)
include_directories(${absl_INCLUDE_DIRS})
target_link_libraries(tokenizer_test PRIVATE
        startear_vm
        startear_tokenizer
        startear_parser
        startear_ast
        startear_program
        gtest
        gtest_main
        pthread
        )
