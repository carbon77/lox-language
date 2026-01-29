cmake_minimum_required(VERSION 3.5)

project(CLox C)

set(SOURCES
  src/main.c
  src/memory.c
  src/chunk.c
  src/debug.c
  src/value.c
  src/vm.c
  src/scanner.c
  src/compiler.c
  src/object.c
  src/table.c
)

configure_file(program.lox src/program.lox COPYONLY)

include_directories(src)

add_executable(CLox ${SOURCES})
