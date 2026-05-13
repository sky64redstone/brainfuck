cmake_minimum_required(VERSION 3.10)

project(brainfuck
  VERSION 0.1.0
  LANGUAGES C
)

# Options & User settings
option(ENABLE_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)

set(CMAKE_C_STANDARD 11)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Build type" FORCE)
endif()

# Brainfuck interpreter
add_executable(bf
  src/main.c
  src/arguments.c
  src/interpreter.c
)

target_include_directories(bf PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Compiler flags
if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wformat=2
    -Wmissing-prototypes
    -Wstrict-prototypes
  )
  if(ENABLE_WARNINGS_AS_ERRORS)
    add_compile_options(-Werror)
  endif()
else()
  message(FATAL_ERROR "Unknown compiler!")
endif()
