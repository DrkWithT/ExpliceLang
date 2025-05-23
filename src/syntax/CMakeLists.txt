add_library(syntax "")
target_include_directories(syntax PUBLIC ${XLANG_INC_DIR})
target_sources(syntax PRIVATE exprs.cpp PRIVATE stmts.cpp)
