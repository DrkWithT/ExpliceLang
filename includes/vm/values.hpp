#pragma once

#include <variant>
#include "codegen/steps.hpp"

namespace XLang::VM {
    enum class ValueTag {
        primitive_null,
        primitive_bool,
        primitive_int,
        primitive_float,
        object_array,
        object_tuple,
        last,
    };

    struct NullValue {};

    // struct FieldReference {
    //     Codegen::Locator location;
    //     int field_id;
    // };

    struct Value {
    public:
        using box_type = std::variant<NullValue, bool, int, float, Codegen::Locator>;

        Value() noexcept;
        explicit Value(bool b);
        explicit Value(int i);
        explicit Value(float f);
        explicit Value(Codegen::Locator ref);

        [[nodiscard]] ValueTag tag() const noexcept;
        [[nodiscard]] const box_type& inner_box() const noexcept;

        [[nodiscard]] bool is_boolean() const noexcept;
        [[nodiscard]] bool is_numeric() const noexcept;
        [[nodiscard]] bool is_object() const noexcept;
        [[nodiscard]] bool is_func_reference() const noexcept;

        // [[nodiscard]] FieldReference access_property(int id) noexcept;
        // [[nodiscard]] FieldReference access_property(std::string_view name) noexcept;

        [[nodiscard]] Value add(const Value& rhs) const;
        [[nodiscard]] Value subtract(const Value& rhs) const;
        [[nodiscard]] Value multiply(const Value& rhs) const;
        [[nodiscard]] Value divide(const Value& rhs) const;
        [[nodiscard]] Value compare_eq(const Value& rhs) const;
        [[nodiscard]] Value compare_ne(const Value& rhs) const;
        [[nodiscard]] Value compare_lt(const Value& rhs) const;
        [[nodiscard]] Value compare_gt(const Value& rhs) const;
        [[nodiscard]] Value logical_and(const Value& rhs) const;
        [[nodiscard]] Value logical_or(const Value& rhs) const;

    private:
        box_type m_data;
        ValueTag m_tag;

    };
}