cmake_minimum_required(VERSION 3.13)

project(LoxLang VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(LoxLang
  src/memory.cpp
  src/chunk.cpp
  src/value.cpp
  src/debug.cpp
  src/vm.cpp
  src/main.cpp
)
