cmake_minimum_required(VERSION 3.30)

project(derkscheme VERSION 0.1.0 LANGUAGES CXX)

set(XLANG_DEMO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test_sources)
set(XLANG_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/includes)
set(XLANG_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(XLANG_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build)

if (DEBUG_MODE)
    add_compile_options(-Wall -Wextra -Wpedantic -Werror -g -Og)
else ()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror -O2)
endif ()

add_subdirectory(${XLANG_SRC_DIR})
enable_testing()
add_subdirectory(tests)
