#pragma once

namespace XLang::VM {
    enum class InstructionArity : unsigned char {
        xia_0,
        xia_1,
        xia_2,
        xia_3
    };

    enum class Opcode : unsigned char {
        xop_halt,
        xop_noop,
        xop_replace,
        xop_push,
        xop_pop,
        xop_peek,
        xop_load_const,
        xop_make_array,
        xop_make_tuple,
        xop_access_field,
        xop_negate,
        xop_add,
        xop_sub,
        xop_mul,
        xop_div,
        xop_cmp_eq,
        xop_cmp_ne,
        xop_cmp_lt,
        xop_cmp_gt,
        xop_log_and,
        xop_log_or,
        xop_jump,
        xop_jump_if,
        xop_jump_not_if,
        xop_ret,
        xop_call,
        xop_call_native,
        last
    };

    enum class Errcode : unsigned char {
        xerr_normal,
        xerr_arithmetic,
        xerr_access,
        xerr_temp_stack,
        xerr_call_stack,
        xerr_heap_error,
        xerr_memory_exceeded,
        xerr_general,
    };

    enum class RoutineType : unsigned char {
        xrt_virtual,
        xrt_native
    };
}