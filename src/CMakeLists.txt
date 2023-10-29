cmake_minimum_required(VERSION 3.22)
project("MarkServer" C)
set(PROJECT_VERSION 0.1)

add_compile_definitions(VERSION="${PROJECT_VERSION}")
add_executable(markserver main.c httpChar.c httpParse.c
        error.c)
