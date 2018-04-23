cmake_minimum_required(VERSION 3.10)
project(Assignment3New)

set(CMAKE_CXX_STANDARD 11)

INCLUDE_DIRECTORIES(/Users/wangjiadong/Downloads/llvm/include/)
LINK_DIRECTORIES(/Users/wangjiadong/Downloads/llvm/lib/)

add_executable(Assignment3New IntervalAnalysis.cpp)