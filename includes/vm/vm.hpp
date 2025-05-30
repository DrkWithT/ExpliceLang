#pragma once

#include <unordered_map>
#include "vm/tags.hpp"
#include "vm/values.hpp"
#include "vm/chunk.hpp"

namespace XLang::VM {
    struct CallFrame {
        ArgStore args;
        int func_id;
    };

    class VM {
    public:
        VM(XpliceProgram prgm) noexcept;

        [[nodiscard]] Errcode run();
        [[nodiscard]] Errcode invoke_virtual_func(const ProgramFunction& func, const ArgStore& args);
        [[nodiscard]] Errcode invoke_native_func(const NativeFunction& func, const ArgStore& args);

        void add_native_function(int native_id, const NativeFunction& func) noexcept;

    private:
        XpliceProgram m_program_funcs;
        std::unordered_map<int, NativeFunction> m_native_funcs;
        std::vector<CallFrame> m_frames;
        std::vector<Value> m_values;
        int m_iptr;
        Errcode m_exit_status;
    };
}