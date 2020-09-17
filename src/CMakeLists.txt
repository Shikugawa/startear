cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

project(startear_library CXX)

include_directories(/usr/local/include)
link_directories(/usr/local/lib)

find_package(absl REQUIRED)

# library
add_library(startear_ast STATIC ast.cpp ast.h)
target_link_directories(startear_ast INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(startear_ast PRIVATE fmt)

add_library(startear_tokenizer STATIC tokenizer.h tokenizer.cpp)
target_include_directories(startear_tokenizer INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_directories(startear_tokenizer PRIVATE startear_ast)

add_library(startear_parser STATIC parser.h parser.cpp)
target_include_directories(startear_parser INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_directories(startear_parser PRIVATE startear_ast startear_tokenizer)

add_library(startear_program STATIC program.h program.cpp)
include_directories(${absl_INCLUDE_DIRS})
target_include_directories(startear_program INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_directories(startear_program PRIVATE startear_ast startear_tokenizer ${absl_LIBRARIES})

add_library(startear_vm STATIC vm_impl.h vm_impl.cpp)
target_include_directories(startear_vm INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_directories(startear_vm INTERFACE startear_program)