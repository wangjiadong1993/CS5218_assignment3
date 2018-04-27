cmake_minimum_required(VERSION 3.10)
project(Assignment_new)

set(CMAKE_CXX_STANDARD 11)

INCLUDE_DIRECTORIES(/Users/wangjiadong/Downloads/llvm/include/)
LINK_DIRECTORIES(/Users/wangjiadong/Downloads/llvm/lib/)

add_executable(Assignment_new DiffAnalysis.cpp DiffAnalysisNew.cpp example2.c example3.c example4.c)