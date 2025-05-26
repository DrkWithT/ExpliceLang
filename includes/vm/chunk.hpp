#pragma once

#include <utility>
#include <unordered_map>
#include <vector>
#include "vm/tags.hpp"
#include "vm/values.hpp"

namespace XLang::VM {
    class VM;

    using RuntimeByte = unsigned char;
    using ArgStore = std::vector<Value>;
    using ConstantStore = std::unordered_map<int, Value>;

    struct Chunk {
        ConstantStore constants;
        std::vector<RuntimeByte> bytecode;
    };

    template <RoutineType RTag>
    class Function {};

    template <>
    class Function <RoutineType::xrt_virtual> {
    private:
        Chunk m_code;

    public:
        Function() = delete;

        Function(Chunk body_code) noexcept
        : m_code {std::move(body_code)} {}

        const Chunk& view_code() const noexcept {
            return m_code;
        }

        template <typename Runtime>
        [[nodiscard]] Errcode invoke(Runtime& vm, const ArgStore& args) const {
            return vm.invoke_virtual_func(*this, args);
        }
    };

    template <>
    class Function <RoutineType::xrt_native> {
    public:
        using native_func_type = Errcode(VM*, const ArgStore&);
        using native_func_ptr = native_func_type*;

    private:
        native_func_ptr m_ptr;

    public:
        Function() = delete;

        Function(native_func_ptr native_func) noexcept
        : m_ptr {native_func} {}

        [[nodiscard]] native_func_ptr ptr() const noexcept {
            return m_ptr;
        }

        template <typename Runtime>
        [[nodiscard]] Errcode invoke(Runtime& vm, const ArgStore& args) const {
            return vm.invoke_native_func(*this, args);
        }
    };
}