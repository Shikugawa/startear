cmake_minimum_required(VERSION 3.16.0)

project(startear_all)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(src)

# main program
add_executable(startear_main main.cpp src/assert.h)
target_link_libraries(startear_main PRIVATE startear_tokenizer)

include_directories(/usr/local/include)
link_directories(/usr/local/lib)

enable_testing()
add_subdirectory(test)

add_test(NAME startear_test COMMAND Test)