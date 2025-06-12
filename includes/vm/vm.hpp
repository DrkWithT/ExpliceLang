#pragma once

#include <array>
#include <unordered_map>
#include "vm/tags.hpp"
#include "vm/values.hpp"
#include "vm/chunk.hpp"

namespace XLang::VM {
    struct CallFrame {
        ArgStore args;
        int callee_id;
        int callee_pos;
        /// @todo use this field based on README plans.
        int callee_frame_base;
    };

    class VM {
    public:
        VM(XpliceProgram prgm) noexcept;

        [[nodiscard]] Errcode run();
        [[nodiscard]] Errcode invoke_native_func(const NativeFunction& func, const ArgStore& args);

        void add_native_function(int native_id, const NativeFunction& func) noexcept;

        const Value& peek_stack_top() const noexcept;
        void push_from_native(Value temp) noexcept;

        void handle_halt();
        void handle_push(const Codegen::Locator& arg);
        void handle_pop(const Codegen::Locator& arg);
        void handle_peek(const Codegen::Locator& arg);

        /* NOTE: implement make_array, make_tuple, access_field... */

        void handle_load_const(const Codegen::Locator& arg);
        void handle_negate();
        void handle_arithmetic(Opcode op);
        void handle_compare(Opcode op);
        void handle_logical(Opcode op);
        void handle_jump_not_if(const Codegen::Locator& arg);
        void handle_return(const Codegen::Locator& arg);
        void handle_call(const Codegen::Locator& local_func_id, int argc);
        void handle_native_call(int module_id, int native_id, int argc);

    private:
        static constexpr std::array<int, static_cast<std::size_t>(Opcode::last)> opcode_arity = {
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

        [[nodiscard]] const CallFrame& current_frame() const noexcept;
        [[nodiscard]] bool is_done() const noexcept;

        [[nodiscard]] Opcode decode_opcode() const noexcept;
        [[nodiscard]] Codegen::Locator decode_arg(int arg_num) const noexcept;

        XpliceProgram m_program_funcs;
        std::unordered_map<int, NativeFunction> m_native_funcs;
        std::vector<CallFrame> m_frames;
        std::vector<Value> m_values;
        int m_iptr;
        Errcode m_exit_status;
    };
}