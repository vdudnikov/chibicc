cmake_minimum_required(VERSION 3.9)
include(CheckLibraryExists)

project(chibicc VERSION 1.0.0 LANGUAGES C)

set(
    SOURCES
    src/hashmap.c
    src/main.c
    src/parse.c
    src/preprocess.c
    src/strings.c
    src/tokenize.c
    src/type.c
    src/unicode.c
    src/codegen/amd64.c
)

# C99
set(CMAKE_C_STANDARD 99)

include_directories(src)
add_executable(chibicc ${SOURCES})
