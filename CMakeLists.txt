cmake_minimum_required(VERSION 3.30)

project(xplicelang VERSION 0.2.0 LANGUAGES CXX)

set(XLANG_DEMO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test_sources)
set(XLANG_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/includes)
set(XLANG_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(XLANG_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build)

if (DEFINED MY_FLAGS)
    add_compile_options(${MY_FLAGS})
else ()
    message (FATAL_ERROR "Preset invalid because it is missing the MY_FLAGS variable.")
endif ()

add_subdirectory(${XLANG_SRC_DIR})
enable_testing()
add_subdirectory(tests)
