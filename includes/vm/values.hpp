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

        // [[nodiscard]] FieldReference access_property(int id) noexcept;
        // [[nodiscard]] FieldReference access_property(std::string_view name) noexcept;

    private:
        box_type m_data;
        ValueTag m_tag;

    };
}