#include <iostream>
#include <print>
#include <string>
#include "frontend/files.hpp"
#include "frontend/lexer.hpp"

using namespace XLang;

struct LexerDump {
    int line;
    int col;
    bool lexing_ok;
};

[[nodiscard]] LexerDump test_lexer(Frontend::Lexer& lexer) noexcept {
    Frontend::Token temp;

    do {
        temp = lexer();

        if (temp.tag == Frontend::LexTag::unknown) {
            break;
        }
    } while (temp.tag != Frontend::LexTag::eof);

    return {
        .line = temp.line,
        .col = temp.column,
        .lexing_ok = temp.tag != Frontend::LexTag::unknown
    };
}

int main(int argc, char* argv[]) {
    if (argc != 2 || !argv[1]) {
        std::print(std::cerr, "Invalid args; Usage: ./xlang_test_lexer <file>\n");
        return 1;
    }

    std::string test_source = Frontend::read_file(argv[1]);
    Frontend::Lexer lexer {test_source};

    if (const auto [fail_line, fail_col, status] = test_lexer(lexer); !status) {
        std::print(std::cerr, "Invalid token lexed in file '{}' at [{}:{}]\n", argv[1], fail_line, fail_col);
        return 1;
    }

    return 0;
}