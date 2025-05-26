#include <array>
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
}
