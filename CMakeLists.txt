cmake_minimum_required(VERSION 4.1)

project(livd C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Wno-unused)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}
)

file(GLOB L_SRCS "src/*.c")
list(APPEND L_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/include/ini/ini.c")

add_executable(livd ${L_SRCS})
target_link_libraries(livd PRIVATE c)
