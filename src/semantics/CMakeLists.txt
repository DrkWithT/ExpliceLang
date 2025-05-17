add_library(semantics "")
target_include_directories(semantics PUBLIC ${XLANG_INC_DIR})
target_sources(semantics PRIVATE tags.cpp)
