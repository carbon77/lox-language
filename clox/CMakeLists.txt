cmake_minimum_required(VERSION 3.0)

project(CLox C)

set(SOURCES
  src/main.c
  src/memory.c
  src/chunk.c
)

include_directories(src)

add_executable(CLox ${SOURCES})
