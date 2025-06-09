# Test lexer...
add_executable(xlang_test_lexer)
target_include_directories(xlang_test_lexer PUBLIC ${XLANG_INC_DIR})
target_link_directories(xlang_test_lexer PRIVATE ${XLANG_LIB_DIR})
target_sources(xlang_test_lexer PRIVATE xlang_test_lexer.cpp)
target_link_libraries(xlang_test_lexer PRIVATE frontend)

add_test(NAME lexer_test_0 COMMAND "$<TARGET_FILE:xlang_test_lexer>" "${XLANG_DEMO_DIR}/test_0.xplice")
add_test(NAME lexer_test_1 COMMAND "$<TARGET_FILE:xlang_test_lexer>" "${XLANG_DEMO_DIR}/test_1.xplice")
add_test(NAME lexer_test_2 COMMAND "$<TARGET_FILE:xlang_test_lexer>" "${XLANG_DEMO_DIR}/test_2.xplice")
add_test(NAME lexer_test_3 COMMAND "$<TARGET_FILE:xlang_test_lexer>" "${XLANG_DEMO_DIR}/test_3.xplice")
add_test(NAME lexer_test_4 COMMAND "$<TARGET_FILE:xlang_test_lexer>" "${XLANG_DEMO_DIR}/test_4.xplice")
add_test(NAME lexer_test_5 COMMAND "$<TARGET_FILE:xlang_test_lexer>" "${XLANG_DEMO_DIR}/test_5.xplice")

# Test parser...
add_executable(xlang_test_parser)
target_include_directories(xlang_test_parser PUBLIC ${XLANG_INC_DIR})
target_link_directories(xlang_test_parser PRIVATE ${XLANG_LIB_DIR})
target_sources(xlang_test_parser PRIVATE xlang_test_parser.cpp)
target_link_libraries(xlang_test_parser PRIVATE frontend PRIVATE syntax)

add_test(NAME parser_test_0 COMMAND "$<TARGET_FILE:xlang_test_parser>" "${XLANG_DEMO_DIR}/test_0.xplice")
add_test(NAME parser_test_1 COMMAND "$<TARGET_FILE:xlang_test_parser>" "${XLANG_DEMO_DIR}/test_1.xplice")
add_test(NAME parser_test_2 COMMAND "$<TARGET_FILE:xlang_test_parser>" "${XLANG_DEMO_DIR}/test_2.xplice")
add_test(NAME parser_test_3 COMMAND "$<TARGET_FILE:xlang_test_parser>" "${XLANG_DEMO_DIR}/test_3.xplice")
# add_test(NAME parser_test_4 COMMAND "$<TARGET_FILE:xlang_test_parser>" "${XLANG_DEMO_DIR}/test_4.xlang")
# add_test(NAME parser_test_5 COMMAND "$<TARGET_FILE:xlang_test_parser>" "${XLANG_DEMO_DIR}/test_5.xlang")

# Test all codegen? MAYBE I should split this up into flow graph generation and actual bytecode gen.
add_executable(xlang_test_codegen)
target_link_directories(xlang_test_codegen PRIVATE "${XLANG_LIB_DIR}")
target_include_directories(xlang_test_codegen PUBLIC "${XLANG_INC_DIR}")
target_link_libraries(xlang_test_codegen PRIVATE frontend PRIVATE semantics PRIVATE syntax PRIVATE codegen PRIVATE vm)
target_sources(xlang_test_codegen PRIVATE xlang_test_codegen.cpp)

add_test(NAME codegen_test_0 COMMAND "$<TARGET_FILE:xlang_test_codegen>" "${XLANG_DEMO_DIR}/test_0.xplice")
add_test(NAME codegen_test_1 COMMAND "$<TARGET_FILE:xlang_test_codegen>" "${XLANG_DEMO_DIR}/test_1.xplice")
add_test(NAME codegen_test_2 COMMAND "$<TARGET_FILE:xlang_test_codegen>" "${XLANG_DEMO_DIR}/test_2.xplice")
add_test(NAME codegen_test_3 COMMAND "$<TARGET_FILE:xlang_test_codegen>" "${XLANG_DEMO_DIR}/test_3.xplice")
# add_test(NAME codegen_test_3b COMMAND "$<TARGET_FILE:xlang_test_codegen>" "${XLANG_DEMO_DIR}/test_4.xlang")
# add_test(NAME codegen_test_3c COMMAND "$<TARGET_FILE:xlang_test_codegen>" "${XLANG_DEMO_DIR}/test_5.xlang")
