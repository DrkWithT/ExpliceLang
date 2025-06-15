#include <utility>
#include <iostream>
#include <print>
#include "frontend/files.hpp"
#include "frontend/parser.hpp"
#include "semantics/analysis.hpp"
#include "codegen/graph_pass.hpp"
#include "codegen/emit_pass.hpp"
#include "vm/chunk.hpp"
#include "vm/vm.hpp"

using namespace XLang;

constexpr auto version_major = 0;
constexpr auto version_minor = 5;
constexpr auto version_patch = 0;

[[nodiscard]] VM::XpliceProgram compile_source(const char* path_cstr) {
    std::string source_str = Frontend::read_file(path_cstr);
    std::string_view source_sv {source_str};

    Frontend::Parser parser {source_sv};

    auto [ast, parse_errors] = parser();

    if (!parse_errors.empty()) {
        std::print("Parse errors of file at {}:\n\n", path_cstr);

        for (const auto& [msg, culprit] : parse_errors) {
            std::print(std::cerr, "Culprit '{}' at [{}:{}]\nNote: {}\n\n", Frontend::peek_lexeme(culprit, source_sv), culprit.line, culprit.column, msg);
        }

        throw std::logic_error {"Compilation failed: parse error(s) found."};
    }

    Semantics::SemanticsPass sema {source_sv};
    auto [sema_native_hints, sema_errors] = sema(ast);

    if (!sema_errors.empty()) {
        std::print(std::cerr, "Semantic errors of file '{}':\n", path_cstr);

        for (const auto& [message, culprit_token] : sema_errors) {
            std::print(std::cerr, "At [ln {}]\nNote: {}\n\n", culprit_token.line, message);
        }

        throw std::logic_error {"Compilation failed: semantic error(s) found."};
    }

    Codegen::GraphPass ir_emitter {source_sv, &sema_native_hints};
    auto [ir_func_constants, ir_func_graphs, ir_main_id] = ir_emitter.process(ast);

    Codegen::EmitCodePass bytecode_emitter;
    auto prgm_ptr = bytecode_emitter.process_full_ir(ir_func_constants, *ir_func_graphs, ir_main_id);

    return {std::move(*prgm_ptr)};
}


[[nodiscard]] VM::Errcode native_print_int(VM::VM* vm_p, const VM::ArgStore& argv) {
    VM::Value target = argv.at(0);
    const auto& target_box = target.inner_box();

    if (!std::holds_alternative<int>(target_box)) {
        vm_p->push_from_native(VM::Value {
            1
        });
        return VM::Errcode::xerr_general;
    }

    std::print("{} ", std::get<int>(target_box));

    vm_p->push_from_native(VM::Value {
        0
    });
    return VM::Errcode::xerr_normal;
}

[[nodiscard]] VM::Errcode native_print_string(VM::VM* vm_p, const VM::ArgStore& argv) {
    const VM::Value& target = argv.at(0);
    const auto& target_box = target.inner_box();

    if (!std::holds_alternative<std::string>(target_box)) {
        vm_p->push_from_native(VM::Value {
            1
        });
        return VM::Errcode::xerr_general;
    }

    std::print("{}\n", std::get<std::string>(target_box));

    vm_p->push_from_native(VM::Value {
        0
    });
    return VM::Errcode::xerr_normal;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::print(std::cerr, "usage: xplice [--help | --version | <source-path>]\n");
        return 1;
    }

    std::string_view process_arg_sv {argv[1]};

    if (process_arg_sv == "--help") {
        std::print(std::cout, "usage: xplice [--help | --version | <source-path>]\n");
        return 0;
    } else if (process_arg_sv == "--version") {
        std::print(std::cout, "Xplice (runtime) v{}.{}.{}\nContributor Link: github.com/DrkWithT\n", version_major, version_minor, version_patch);
        return 0;
    }

    VM::NativeFunction wrap_print_int {native_print_int};
    VM::NativeFunction wrap_print_string {native_print_string};

    try {
        /// 1. Initialize VM...
        VM::VM engine {compile_source(argv[1])};

        /// 2. Register a print function for convenience...
        engine.add_native_function(0, wrap_print_int);
        engine.add_native_function(1, wrap_print_string);

        auto error_status = engine.run();

        if (error_status != VM::Errcode::xerr_normal) {
            std::print(std::cerr, "Xplice program exited with status code {}\n", static_cast<unsigned int>(error_status));
            return 1;
        }
    } catch (const std::logic_error& compile_error) {
        std::print(std::cerr, "Compile Error:\n{}\n", compile_error.what());
        return 1;
    } catch (const std::runtime_error& vm_error) {
        std::print(std::cerr, "RuntimeError:\n{}\n", vm_error.what());
        return 1;
    }
}