add_library(vm "")
target_include_directories(vm PUBLIC ${XLANG_INC_DIR})
target_sources(vm PRIVATE values.cpp PRIVATE vm.cpp)
