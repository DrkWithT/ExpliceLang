add_library(vm "")
target_include_directories(vm PUBLIC ${XLANG_INC_DIR})
target_sources(vm PRIVATE values.cpp) # TODO: add PRIVATE vm.cpp when done.
