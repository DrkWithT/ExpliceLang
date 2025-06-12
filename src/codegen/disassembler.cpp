#include <iostream>
#include <print>
#include "codegen/disassembler.hpp"

namespace XLang::Codegen {
    void Disassembler::operator()(const VM::XpliceProgram& program) {
        const auto& [program_chunks, program_main_id] = program;

        for (const auto& [func_id, func_chunk] : program_chunks) {
            print_chunk(func_id, program_main_id, func_chunk.view_code());
        }
    }

    void Disassembler::print_chunk(int chunk_func_id, int main_func_id, const VM::Chunk& chunk) {
        const auto& bytecode = chunk.bytecode;
        const std::size_t chunk_size = chunk.bytecode.size();
        std::size_t chunk_pos = 0;

        auto print_opcode = [&bytecode, &chunk_pos]() {
            const auto opcode_id = bytecode[chunk_pos];
            const auto opcode_arity = cm_opcode_arities.at(opcode_id);

            std::print("{} ", cm_opcode_names.at(opcode_id));
            ++chunk_pos;

            return opcode_arity;
        };

        auto print_arg = [&bytecode, &chunk_pos]() {
            const auto arg_region_id = bytecode[chunk_pos];
            ++chunk_pos;
            auto arg_num = 0;

            arg_num += bytecode[chunk_pos];
            arg_num += (bytecode[chunk_pos + 1]) << 8;
            arg_num += (bytecode[chunk_pos + 2]) << 16;
            arg_num += (bytecode[chunk_pos + 3]) << 24;
            chunk_pos += 4;

            std::print("{}:{} ", cm_region_names.at(arg_region_id), arg_num);
        };

        if (chunk_func_id == main_func_id) {
            std::print(std::cout, "Function Chunk (main):\n\n");
        } else {
            std::print(std::cout, "Function Chunk {}:\n\n", chunk_func_id);
        }

        while (chunk_pos < chunk_size) {
            std::print("{}: ", chunk_pos);
            const auto op_arity = print_opcode();

            switch (op_arity) {
            case 0:
                break;
            case 1:
                print_arg();
                break;
            case 2:
                print_arg();
                print_arg();
                break;
            case 3:
                print_arg();
                print_arg();
                print_arg();
                break;
            default:
                std::print("...");
                break;
            }

            std::print("\n");
        }
    }
}
