#include <iostream>
#include <print>
#include "frontend/files.hpp"
#include "frontend/parser.hpp"
#include "codegen/graph_pass.hpp"
#include "codegen/graph_printer.hpp"

using namespace XLang;

[[nodiscard]] constexpr bool test_codegen_on(std::string_view source_view, std::string_view file_name) {
    Frontend::Parser parser {source_view};

    auto parse_result = parser();

    /// Print any parsing errors for debugging purposes.
    if (const auto& parse_errors = parse_result.errors; !parse_errors.empty()) {
        std::print(std::cerr, "For file '{}':\n", file_name);

        for (const auto& [msg, culprit] : parse_errors) {
            std::print(std::cerr, "Syntax Error:\nCulprit '{}' at [{}:{}]\nNote: {}\n\n", Frontend::peek_lexeme(culprit, source_view), culprit.line, culprit.column, msg);
        }

        return false;
    }

    Codegen::FlowGraphPrinter printer;
    Codegen::GraphPass gen_graph_pass {source_view};
    auto graph_ptr = gen_graph_pass.process(parse_result.decls);

    printer(*graph_ptr);

    return true;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::print(std::cerr, "usage: ./xlang_test_codegen <test-source-file>\n");
        return 1;
    }

    std::string source = Frontend::read_file(argv[1]);

    if (!test_codegen_on(source, argv[1])) {
        return 1;
    }
}