#pragma once

#include "vm/tags.hpp"

namespace XLang::Codegen {
    enum class Region : unsigned char {
        consts,
        temp_stack,
        obj_heap,
        routines,
        frame_slot,
        none,
        last
    };

    struct Locator {
        Region region;
        int id;
    };

    struct NonaryStep {
        VM::Opcode op;
    };

    struct UnaryStep {
        VM::Opcode op;
        Locator arg_0;
    };

    struct BinaryStep {
        VM::Opcode op;
        Locator arg_0;
        Locator arg_1;
    };

    struct TernaryStep {
        VM::Opcode op;
        Locator arg_0;
        Locator arg_1;
        Locator arg_2;
    };
}