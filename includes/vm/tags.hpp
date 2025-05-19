#pragma once

namespace XLang::VM {
    enum class Opcode : unsigned char {
        xop_halt,
        xop_noop,
        xop_push_temp,
        xop_pop_temp,
        xop_put_const,
        xop_put_value,
        xop_make_array,
        xop_make_tuple,
        xop_get_slot,
        xop_set_slot,
        xop_get_item,
        xop_add_int,
        xop_add_float,
        xop_sub_int,
        xop_sub_float,
        xop_mul_int,
        xop_mul_float,
        xop_div_int,
        xop_div_float,
        xop_cmp_eq,
        xop_cmp_ne,
        xop_cmp_lt,
        xop_cmp_gt,
        xop_jump,
        xop_jump_if,
        xop_create_frame,
        xop_leave_frame,
        xop_call,
        xop_call_native
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
}