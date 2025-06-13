#include <stdexcept>
#include <utility>
#include "vm/vm.hpp"

namespace XLang::VM {
    static constexpr auto opcode_stride = 1;
    static constexpr auto opcode_arg_stride = 5;
    static constexpr Codegen::Locator placeholder_arg {Codegen::Region::none, -1}; 

    static auto decode_i32 = [] [[nodiscard]] (const std::vector<RuntimeByte>& code_buffer, int position) noexcept {
        const auto result_0 = code_buffer[position] & 0x000000ff;
        const auto result_1 = (code_buffer[position + 1] & 0x0000ff00) >> 8;
        const auto result_2 = (code_buffer[position + 2] & 0x00ff0000) >> 16;
        const auto result_3 = (code_buffer[position + 3] & 0xff000000) >> 24;

        return result_0 + result_1 + result_2 + result_3;
    };

    VM::VM(XpliceProgram prgm) noexcept
    : m_program_funcs {std::move(prgm)}, m_native_funcs {}, m_frames {}, m_values {}, m_iptr {0}, m_exit_status {Errcode::xerr_normal} {
        /// NOTE: VM starts execution at main function / entry point... place main on the stack as a base for the call frame values.
        m_values.emplace_back(Value {Codegen::Locator {
            .region = Codegen::Region::routines,
            .id = m_program_funcs.entry_func_id
        }});

        m_frames.emplace_back(
            ArgStore {},
            m_program_funcs.entry_func_id,
            m_iptr,
            0
        );
    }

    Errcode VM::run() {
        auto op = Opcode::xop_noop;
        auto op_arity = 0;
        std::array<Codegen::Locator, 3> op_args = {
            placeholder_arg,
            placeholder_arg,
            placeholder_arg
        };

        while (!is_done()) {
            op = decode_opcode();
            op_arity = opcode_arity[static_cast<unsigned int>(op)];

            switch (op_arity) {
            case 1:
                op_args[0] = decode_arg(0);
                break;
            case 2:
                op_args[0] = decode_arg(0);
                op_args[1] = decode_arg(1);
                break;
            case 3:
                op_args[0] = decode_arg(0);
                op_args[1] = decode_arg(1);
                op_args[2] = decode_arg(2);
                break;    
            case 0:
            default:
                break;
            }

            switch (op) {
            case Opcode::xop_halt:
                m_exit_status = Errcode::xerr_general;
                throw std::runtime_error {"Reached premature halt!"};
                break;
            case Opcode::xop_noop:
                ++m_iptr;
                break;
            case Opcode::xop_replace:
                handle_replace(op_args[0]);
                m_iptr += 6;
                break;
            case Opcode::xop_push:
                handle_push(op_args[0]);
                m_iptr += 6; // stride of unary instruction
                break;
            case Opcode::xop_pop:
                handle_pop(op_args[0]);
                m_iptr += 6;
                break;
            case Opcode::xop_peek:
                handle_peek(op_args[0]);
                m_iptr += 6;
                break;
            case Opcode::xop_load_const:
                handle_load_const(op_args[0]);
                m_iptr += 6;
                break;
            case Opcode::xop_make_array:
            case Opcode::xop_make_tuple:
            case Opcode::xop_access_field:
                m_exit_status = Errcode::xerr_general;
                throw std::runtime_error {"Unsupported opcode."};
                break;
            case Opcode::xop_negate:
                handle_negate();
                ++m_iptr; // stride of no-arg instruction
                break;
            case Opcode::xop_add:
            case Opcode::xop_sub:
            case Opcode::xop_mul:
            case Opcode::xop_div:
                handle_arithmetic(op);
                ++m_iptr;
                break;
            case Opcode::xop_cmp_eq:
            case Opcode::xop_cmp_ne:
            case Opcode::xop_cmp_lt:
            case Opcode::xop_cmp_gt:
                handle_compare(op);
                ++m_iptr;
                break;
            case Opcode::xop_log_and:
            case Opcode::xop_log_or:
                handle_logical(op);
                ++m_iptr;
                break;
            case Opcode::xop_jump:
                m_iptr = op_args[0].id;
                break;
            case Opcode::xop_jump_not_if:
                handle_jump_not_if(op_args[0]);
                break;
            case Opcode::xop_ret:
                handle_return(op_args[0]);
                break;
            case Opcode::xop_call:
                handle_call(op_args[0], op_args[1].id);
                break;
            case Opcode::xop_call_native:
                handle_native_call(op_args[0].id, op_args[1].id, op_args[2].id);
                break;
            default:
                m_exit_status = Errcode::xerr_general;
                throw std::runtime_error {"Illegal opcode read."};
                break;
            }
        }

        if (m_exit_status == Errcode::xerr_normal) {
            const auto main_ret = std::get<int>(m_values.front().inner_box());

            m_exit_status = (main_ret == 0) ? Errcode::xerr_normal : Errcode::xerr_general;
        }

        return m_exit_status;
    }

    Errcode VM::invoke_native_func(const NativeFunction& func, const ArgStore& args) {
        return func.ptr()(this, args);
    }

    /// @note Registers a native function wrapper to the runtime. The `id` must ascend from 0 to N corresponding to the order of use-native statements!
    void VM::add_native_function(int native_id, const NativeFunction& func) noexcept {
        m_native_funcs[native_id] = func;
    }


    const CallFrame& VM::current_frame() const noexcept {
        return m_frames.back();
    }

    bool VM::is_done() const noexcept {
        return m_frames.empty();
    }

    Opcode VM::decode_opcode() const noexcept {
        // std::cout << "CHUNK: " << current_frame().callee_id <<  ", IPTR: " << m_iptr << '\n';
        const auto decoded_op = m_program_funcs.func_chunks.at(current_frame().callee_id).view_code().bytecode[m_iptr];

        return static_cast<Opcode>(decoded_op);
    }

    Codegen::Locator VM::decode_arg(int arg_num) const noexcept {
        const auto& chunk_view = m_program_funcs.func_chunks.at(current_frame().callee_id).view_code().bytecode;
        const auto arg_byte_pos = opcode_stride + (arg_num * opcode_arg_stride);

        return {
            .region = static_cast<Codegen::Region>(
                chunk_view[m_iptr + arg_byte_pos]
            ),
            .id = static_cast<int>(
                decode_i32(chunk_view, m_iptr + arg_byte_pos + 1)
            )
        };
    }

    const Value& VM::peek_stack_top() const noexcept {
        return m_values.back();
    }

    void VM::push_from_native(Value temp) noexcept {
        m_values.emplace_back(std::move(temp));
    }

    void VM::handle_halt() {
        m_frames.clear();
    }

    void VM::handle_replace(const Codegen::Locator& arg) {
        if (arg.region != Codegen::Region::temp_stack) {
            m_exit_status = Errcode::xerr_temp_stack;
            throw std::runtime_error {"Illegal argument to REPLACE instruction detected - not a stack locator."};
        }

        m_values[current_frame().callee_frame_base + arg.id] = m_values.back();
        m_values.pop_back();
    }

    void VM::handle_push(const Codegen::Locator& arg) {
        switch (arg.region) {
        case Codegen::Region::consts:
            m_values.push_back(
                m_program_funcs.func_chunks.at(
                    current_frame().callee_id
                ).view_code().constants.at(arg.id)
            );
            break;
        case Codegen::Region::temp_stack:
            m_values.push_back(m_values[current_frame().callee_frame_base + arg.id]);
            break;
        case Codegen::Region::obj_heap:
            m_exit_status = Errcode::xerr_temp_stack;
            throw std::runtime_error {"Unimplemented push of heap object."};
            break;
        case Codegen::Region::routines:
            m_values.push_back(Value {arg});
            break;
        case Codegen::Region::frame_slot:
            m_values.push_back(current_frame().args[arg.id]);
            break;
        case Codegen::Region::none:
        default:
            m_exit_status = Errcode::xerr_temp_stack;
            throw std::runtime_error {"Illegal push of un-tagged object detected."};
            break;
        }
    }

    void VM::handle_pop(const Codegen::Locator& arg) {
        for (auto pop_count = 0; pop_count < arg.id; ++pop_count) {
            m_values.pop_back();
        }
    }

    void VM::handle_peek(const Codegen::Locator& arg) {
        handle_push(arg);
    }

    void VM::handle_load_const(const Codegen::Locator& arg) {
        m_values.push_back(
            m_program_funcs.func_chunks.at(
                current_frame().callee_id
            ).view_code().constants.at(arg.id)
        );
    }

    void VM::handle_negate() {
        auto arg = m_values.back();
        m_values.pop_back();

        if (arg.is_numeric()) {   
            const auto& arg_boxed = arg.inner_box();

            if (arg_boxed.index() == 2) {
                m_values.emplace_back(-std::get<int>(arg_boxed));
            } else if (arg_boxed.index() == 3) {
                m_values.emplace_back(-std::get<float>(arg_boxed));
            }

            return;
        }

        m_exit_status = Errcode::xerr_arithmetic;
        throw std::runtime_error {"Invalid negation on non-numeric Value."};
    }

    void VM::handle_arithmetic(Opcode op) {
        Value arg_lhs;
        Value arg_rhs;

        arg_lhs = m_values.back();
        m_values.pop_back();
        arg_rhs = m_values.back();
        m_values.pop_back();

        if (op == Opcode::xop_add) {
            m_values.emplace_back(arg_lhs.add(arg_rhs));
        } else if (op == Opcode::xop_mul) {
            m_values.emplace_back(arg_lhs.multiply(arg_rhs));
        } else if (op == Opcode::xop_sub) {
            m_values.emplace_back(arg_lhs.subtract(arg_rhs));
        } else {
            m_values.emplace_back(arg_lhs.divide(arg_rhs));
        }
    }

    void VM::handle_compare(Opcode op) {
        Value lhs_box = m_values.back();
        m_values.pop_back();
        Value rhs_box = m_values.back();
        m_values.pop_back();

        if (op == Opcode::xop_cmp_eq) {
            m_values.emplace_back(lhs_box.compare_eq(rhs_box));
        } else if (op == Opcode::xop_cmp_ne) {
            m_values.emplace_back(lhs_box.compare_ne(rhs_box));
        } else if (op == Opcode::xop_cmp_lt) {
            m_values.emplace_back(lhs_box.compare_lt(rhs_box));
        } else {
            m_values.emplace_back(lhs_box.compare_gt(rhs_box));
        }
    }

    void VM::handle_logical(Opcode op) {
        Value lhs_box = m_values.back();
        m_values.pop_back();
        Value rhs_box = m_values.back();
        m_values.pop_back();

        if (op == Opcode::xop_log_and) {
            m_values.emplace_back(lhs_box.logical_and(rhs_box));
        } else {
            m_values.emplace_back(lhs_box.logical_or(rhs_box));
        }
    }

    void VM::handle_jump_not_if(const Codegen::Locator& arg) {
        Value check_val = m_values.back();
        m_values.pop_back();

        if (!std::get<bool>(check_val.inner_box())) {
            m_iptr = arg.id;
        } else {
            m_iptr += 6; // advance past jump_if with encoded stride of 6 bytes
        }
    }

    void VM::handle_return(const Codegen::Locator& arg) {
        const auto [arg_tag, arg_num] = arg;
        Value result = ([&arg, this](Codegen::Region tag, int num) {
            switch (tag) {
            case Codegen::Region::consts:
                return m_program_funcs.func_chunks.at(
                    current_frame().callee_id
                ).view_code().constants.at(num);
            case Codegen::Region::temp_stack:
                return m_values[current_frame().callee_frame_base + num];
            case Codegen::Region::obj_heap:
            case Codegen::Region::routines:
                return Value {arg};
            case Codegen::Region::frame_slot:
                return current_frame().args[num];
            case Codegen::Region::none:
                return m_values.back();
            default:
                m_exit_status = Errcode::xerr_temp_stack;
                throw std::runtime_error {"Invalid argument tag for opcode RET: none, etc."};
                break;
            }
        })(arg_tag, arg_num); // Use IIFE here for conditional multi-value setting.

        while (!m_values.back().is_func_reference()) {
            m_values.pop_back();
        }
        m_values.pop_back();

        m_values.emplace_back(std::move(result));

        m_frames.pop_back();

        if (!is_done()) {
            m_iptr = current_frame().callee_pos;
        } else {
            m_iptr = 0;
        }
    }

    void VM::handle_call(const Codegen::Locator& local_func_id, int argc) {
        /// NOTE: store return address in caller before entering callee...
        const auto ret_func_pos = m_iptr + opcode_stride + (2 * opcode_arg_stride);
        m_frames.back().callee_pos = ret_func_pos;

        ArgStore args;

        for (auto arg_count = 0; arg_count < argc; ++arg_count) {
            auto temp_arg = m_values.back();
            args.emplace_back(std::move(temp_arg));
            m_values.pop_back();
        }

        /// NOTE: prepare base value & call state of function frame...
        const auto base_mark = static_cast<int>(m_values.size());
        m_values.emplace_back(Value {Codegen::Locator {
            .region = Codegen::Region::routines,
            .id = local_func_id.id
        }});

        m_frames.emplace_back(CallFrame {
            .args = std::move(args),
            .callee_id = local_func_id.id,
            .callee_pos = 0,
            .callee_frame_base = base_mark
        });

        /// NOTE: resume execution at the beginning of the callee's chunk
        m_iptr = 0;
    }

    void VM::handle_native_call([[maybe_unused]] int module_id, int native_id, int argc) {
        ArgStore args;

        for (auto arg_count = 0; arg_count < argc; ++arg_count) {
            auto temp_arg = m_values.back();
            args.emplace_back(std::move(temp_arg));
            m_values.pop_back();
        }

        m_exit_status = m_native_funcs.at(native_id).invoke(*this, args);

        /// NOTE: resume execution after native_call instruction (16B encoded)
        m_iptr += opcode_stride + (3 * opcode_arg_stride);
    }
}
