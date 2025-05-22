add_subdirectory(frontend)
add_subdirectory(syntax)
add_subdirectory(semantics)
add_subdirectory(codegen)
# add_subdirectory(vm) # todo

add_executable(xlang)
target_include_directories(xlang PUBLIC ${XLANG_INC_DIR})
target_link_directories(xlang PRIVATE ${XLANG_LIB_DIR})
# target_link_libraries(xlang PRIVATE frontend PRIVATE syntax PRIVATE semantics PRIVATE codegen PRIVATE vm)
target_sources(xlang PRIVATE xlang.cpp)
