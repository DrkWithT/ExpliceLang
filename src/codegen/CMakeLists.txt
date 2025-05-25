add_library(codegen "")
target_include_directories(codegen PUBLIC ${XLANG_INC_DIR})
target_sources(codegen PRIVATE flow_nodes.cpp PRIVATE ir_printer.cpp PRIVATE graph_pass.cpp)
