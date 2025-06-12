#include <iostream>
#include <print>
#include "frontend/files.hpp"
#include "frontend/parser.hpp"
#include "semantics/analysis.hpp"
#include "codegen/graph_pass.hpp"
#include "codegen/ir_printer.hpp"
#include "codegen/emit_pass.hpp"
#include "codegen/disassembler.hpp"

using namespace XLang;

[[nodiscard]] constexpr bool test_codegen_on(std::string_view source_view, std::string_view file_name) {
    Frontend::Parser parser {source_view};

    auto [ast , parse_errors] = parser();

    /// Print any parsing errors for debugging purposes.
    if (!parse_errors.empty()) {
        std::print(std::cerr, "For file '{}':\n", file_name);

        for (const auto& [msg, culprit] : parse_errors) {
            std::print(std::cerr, "Syntax Error:\nCulprit '{}' at [{}:{}]\nNote: {}\n\n", Frontend::peek_lexeme(culprit, source_view), culprit.line, culprit.column, msg);
        }

        return false;
    }

    Semantics::SemanticsPass sema {source_view};
    auto [sema_native_hints, sema_errors] = sema(ast);

    if (!sema_errors.empty()) {
        std::print(std::cerr, "Semantic errors in file '{}':\n", file_name);

        for (const auto& [message, culprit_token] : sema_errors) {
            std::print(std::cerr, "At [ln {}]\nNote: {}\n\n", culprit_token.line, message);
        }

        return false;
    }

    Codegen::IRPrinter printer;
    Codegen::GraphPass gen_graph_pass {source_view, &sema_native_hints};
    auto ir = gen_graph_pass.process(ast);

    printer(ir);

    const auto& [constants_storage, cfg_map_sp, main_id] = ir;

    Codegen::EmitCodePass emitter;
    auto foo = emitter.process_full_ir(constants_storage, *cfg_map_sp, main_id);

    Codegen::Disassembler disassembler;
    disassembler(*foo);

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