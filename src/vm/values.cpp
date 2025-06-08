#include <array>
#include <iostream>
#include <stdexcept>
#include "vm/values.hpp"

namespace XLang::VM {
    constexpr std::array<ValueTag, static_cast<std::size_t>(ValueTag::last)> value_tags_table = {
        ValueTag::primitive_null,
        ValueTag::primitive_bool,
        ValueTag::primitive_int,
        ValueTag::primitive_float,
        ValueTag::object_array,
        ValueTag::object_tuple
    };

    Value::Value() noexcept
    : m_data {NullValue {}}, m_tag {value_tags_table[m_data.index()]} {}

    Value::Value(bool b)
    : m_data {b}, m_tag {value_tags_table[m_data.index()]} {}

    Value::Value(int i)
    : m_data {i}, m_tag {value_tags_table[m_data.index()]} {}

    Value::Value(float f)
    : m_data {f}, m_tag {value_tags_table[m_data.index()]} {}

    Value::Value(Codegen::Locator ref)
    : m_data {ref}, m_tag {value_tags_table[m_data.index()]} {}


    ValueTag Value::tag() const noexcept {
        return m_tag;
    }

    const Value::box_type& Value::inner_box() const noexcept {
        return m_data;
    }

    bool Value::is_boolean() const noexcept {
        return m_tag == ValueTag::primitive_bool;
    }

    bool Value::is_numeric() const noexcept {
        return m_tag == ValueTag::primitive_int || m_tag == ValueTag::primitive_float;
    }

    bool Value::is_object() const noexcept {
        return m_tag == ValueTag::object_array || m_tag == ValueTag::object_tuple;
    }

    bool Value::is_func_reference() const noexcept {
        if (m_data.index() != 4) {
            return false; // handle non-reference types...
        }

        return std::get<Codegen::Locator>(m_data).region == Codegen::Region::routines;
    }

    Value Value::add(const Value& rhs) const {
        if (!is_numeric() || !rhs.is_numeric()) {
            throw std::runtime_error {"Invalid add operands: NaN detected."};
        }

        if (m_tag != rhs.tag()) {
            throw std::runtime_error {"Invalid add operands: mismatched types."};
        }

        const auto lhs_tag = tag();

        if (lhs_tag == ValueTag::primitive_int) {
            return Value {std::get<int>(m_data) + std::get<int>(rhs.m_data)};
        } else {
            return Value {std::get<float>(m_data) + std::get<float>(m_data)};
        }
    }

    Value Value::subtract(const Value& rhs) const {
        if (!is_numeric() || !rhs.is_numeric()) {
            std::cout << "lhs-tag: " << m_data.index() << ", rhs-tag: " << rhs.m_data.index() << '\n'; // debug
            throw std::runtime_error {"Invalid add operands: NaN detected."};
        }

        if (m_tag != rhs.tag()) {
            throw std::runtime_error {"Invalid add operands: mismatched types."};
        }

        const auto lhs_tag = tag();

        if (lhs_tag == ValueTag::primitive_int) {
            return Value {std::get<int>(m_data) - std::get<int>(rhs.m_data)};
        } else {
            return Value {std::get<float>(m_data) - std::get<float>(m_data)};
        }
    }

    Value Value::multiply(const Value& rhs) const {
        if (!is_numeric() || !rhs.is_numeric()) {
            throw std::runtime_error {"Invalid add operands: NaN detected."};
        }

        if (m_tag != rhs.tag()) {
            throw std::runtime_error {"Invalid add operands: mismatched types."};
        }

        const auto lhs_tag = tag();

        if (lhs_tag == ValueTag::primitive_int) {
            return Value {std::get<int>(m_data) * std::get<int>(rhs.m_data)};
        } else {
            return Value {std::get<float>(m_data) * std::get<float>(m_data)};
        }
    }

    Value Value::divide(const Value& rhs) const {
        if (!is_numeric() || !rhs.is_numeric()) {
            throw std::runtime_error {"Invalid add operands: NaN detected."};
        }

        if (m_tag != rhs.tag()) {
            throw std::runtime_error {"Invalid add operands: mismatched types."};
        }

        const auto lhs_tag = tag();

        if (lhs_tag == ValueTag::primitive_int) {
            const auto rhs_val = std::get<int>(rhs.m_data);

            if (rhs_val == 0) {
                throw std::runtime_error {"Cannot divide by zero!"};
            }

            return Value {std::get<int>(m_data) / rhs_val};
        } else {
            const auto rhs_val = std::get<float>(rhs.m_data);

            if (rhs_val == 0.0f) {
                throw std::runtime_error {"Cannot divide by zero!"};
            }

            return Value {std::get<float>(m_data) / rhs_val};
        }
    }

    Value Value::compare_eq(const Value& rhs) const {
        if (m_tag != rhs.tag()) {
            return Value {false};
        }

        const auto type_tag = m_tag;

        if (type_tag == ValueTag::primitive_null) {
            return Value {true};
        } else if (type_tag == ValueTag::primitive_bool) {
            return Value {std::get<bool>(m_data) == std::get<bool>(rhs.m_data)};
        } else if (type_tag == ValueTag::primitive_int) {
            return Value {std::get<int>(m_data) == std::get<int>(rhs.m_data)};
        } else if (type_tag == ValueTag::primitive_float) {
            return Value {std::get<float>(m_data) == std::get<float>(rhs.m_data)};
        } else {
            throw std::runtime_error {"Unsupported operation for array / tuple."};
        }
    }

    Value Value::compare_ne(const Value& rhs) const {
        if (m_tag != rhs.tag()) {
            return Value {false};
        }

        const auto type_tag = m_tag;

        if (type_tag == ValueTag::primitive_null) {
            return Value {true};
        } else if (type_tag == ValueTag::primitive_bool) {
            return Value {std::get<bool>(m_data) != std::get<bool>(rhs.m_data)};
        } else if (type_tag == ValueTag::primitive_int) {
            return Value {std::get<int>(m_data) != std::get<int>(rhs.m_data)};
        } else if (type_tag == ValueTag::primitive_float) {
            return Value {std::get<float>(m_data) != std::get<float>(rhs.m_data)};
        } else {
            throw std::runtime_error {"Unsupported operation for array / tuple."};
        }
    }

    Value Value::compare_lt(const Value& rhs) const {
        if (m_tag != rhs.tag()) {
            return Value {false};
        }

        const auto type_tag = m_tag;

        if (type_tag == ValueTag::primitive_null) {
            return Value {true};
        } else if (type_tag == ValueTag::primitive_bool) {
            return Value {std::get<bool>(m_data) < std::get<bool>(rhs.m_data)};
        } else if (type_tag == ValueTag::primitive_int) {
            const auto temp_lhs = std::get<int>(m_data);
            const auto temp_rhs = std::get<int>(rhs.m_data);
            return Value {temp_lhs < temp_rhs};
        } else if (type_tag == ValueTag::primitive_float) {
            return Value {std::get<float>(m_data) < std::get<float>(rhs.m_data)};
        } else {
            throw std::runtime_error {"Unsupported operation for array / tuple."};
        }
    }

    Value Value::compare_gt(const Value& rhs) const {
        if (m_tag != rhs.tag()) {
            return Value {false};
        }

        const auto type_tag = m_tag;

        if (type_tag == ValueTag::primitive_null) {
            return Value {true};
        } else if (type_tag == ValueTag::primitive_bool) {
            return Value {std::get<bool>(m_data) > std::get<bool>(rhs.m_data)};
        } else if (type_tag == ValueTag::primitive_int) {
            return Value {std::get<int>(m_data) > std::get<int>(rhs.m_data)};
        } else if (type_tag == ValueTag::primitive_float) {
            return Value {std::get<float>(m_data) > std::get<float>(rhs.m_data)};
        } else {
            throw std::runtime_error {"Unsupported operation for array / tuple."};
        }
    }

    Value Value::logical_and(const Value& rhs) const {
        if (!is_boolean() || !rhs.is_boolean()) {
            throw std::runtime_error {"Logical AND unsupported for non-booleans."};
        }

        return Value {std::get<bool>(m_data) && std::get<bool>(rhs.m_data)};
    }

    Value Value::logical_or(const Value& rhs) const {
        if (!is_boolean() || !rhs.is_boolean()) {
            throw std::runtime_error {"Logical OR unsupported for non-booleans."};
        }

        return Value {std::get<bool>(m_data) || std::get<bool>(rhs.m_data)};
    }
}
