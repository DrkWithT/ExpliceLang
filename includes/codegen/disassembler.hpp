#pragma once

#include <string_view>
#include <array>
#include "vm/tags.hpp"
#include "vm/chunk.hpp"

namespace XLang::Codegen {
    class Disassembler {
    public:
        Disassembler() noexcept = default;

        void operator()(const VM::XpliceProgram& program);

    private:
        void print_chunk(int chunk_func_id, int main_func_id, const VM::Chunk& chunk);

        static constexpr std::array<std::string_view, static_cast<std::size_t>(VM::Opcode::last)> cm_opcode_names = {
            "halt",
            "noop",
            "push",
            "pop",
            "peek",
            "load_const",
            "make_array",
            "make_tuple",
            "access_field",
            "negate",
            "add",
            "sub",
            "mul",
            "div",
            "cmp_eq",
            "cmp_ne",
            "cmp_lt",
            "cmp_gt",
            "log_and",
            "log_or",
            "jump",
            "jump_if",
            "jump_not_if",
            "ret",
            "call",
            "call_native"
        };

        static constexpr std::array<int, static_cast<std::size_t>(VM::Opcode::last)> cm_opcode_arities = {
            0,
            0,
            1,
            1,
            1,
            1,
            1,
            1,
            2,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            1,
            1,
            1,
            1,
            2,
            3
        };

        static constexpr std::array<std::string_view, static_cast<std::size_t>(Region::last)> cm_region_names = {
            "consts",
            "stack",
            "heap",
            "routines",
            "natives",
            "frame_slot",
            "none",
        };
    };
}