#include <iostream>
#include <print>
#include <string_view>
#include <string>
#include "frontend/files.hpp"
#include "frontend/parser.hpp"

using namespace XLang;

[[nodiscard]] bool test_parse_on(std::string_view source_view, std::string_view file_name) {
    Frontend::Parser parser {source_view};

    auto parse_result = parser();

    if (const auto& parse_errors = parse_result.errors; !parse_errors.empty()) {
        std::print(std::cerr, "For file '{}':\n", file_name);

        for (const auto& [msg, culprit] : parse_errors) {
            std::print(std::cerr, "Syntax Error:\nCulprit '{}' at [{}:{}]\nNote: {}\n\n", Frontend::peek_lexeme(culprit, source_view), culprit.line, culprit.column, msg);
        }

        return false;
    }

    return true;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::print(std::cerr, "usage: ./xlang_test_parser <test-source-file>\n");
        return 1;
    }

    std::string source = Frontend::read_file(argv[1]);

    if (!test_parse_on(source, argv[1])) {
        return 1;
    }
}
