cmake_minimum_required(VERSION 3.0)

project(CLox C)

set(SOURCES
  src/main.c
  src/memory.c
  src/chunk.c
  src/debug.c
  src/value.c
  src/vm.c
)

include_directories(src)

add_executable(CLox ${SOURCES})
