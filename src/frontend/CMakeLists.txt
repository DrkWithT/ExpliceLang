add_library(frontend "")
target_include_directories(frontend PUBLIC ${XLANG_INC_DIR})
target_sources(frontend PRIVATE files.cpp PRIVATE token.cpp PRIVATE lexer.cpp)
