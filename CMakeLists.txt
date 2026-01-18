cmake_minimum_required(VERSION 4.1)
project(c_amx C ASM)

set(CMAKE_C_STANDARD 17)

add_executable(c_amx
    src/main.c
    src/amx_util.c
    src/amx_asm.S
)
target_include_directories(c_amx PRIVATE src ${CMAKE_SOURCE_DIR})

add_executable(c_amx_tests
    tests/test_ldst.c
    src/amx_util.c
    src/amx_asm.S
)
target_include_directories(c_amx_tests PRIVATE src ${CMAKE_SOURCE_DIR})
