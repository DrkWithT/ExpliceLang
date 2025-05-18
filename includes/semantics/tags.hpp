#pragma once

// #include <vector>
#include <string>

namespace XLang::Semantics {
    enum class OpTag : unsigned char {
        none,
        assign,
        access,
        negate,
        multiply,
        divide,
        add,
        subtract,
        cmp_equ,
        cmp_neq,
        cmp_lt,
        cmp_gt,
        logic_and,
        logic_or
    };

    enum class ValuingTag : unsigned char {
        x_in_value, // anything mutable
        x_out_value, // primitive, constant, or read-only
        x_unknown_value
    };

    enum class TypeTag : unsigned char {
        x_type_bool,
        x_type_int,
        x_type_float,
        x_type_string,
        x_type_unknown
    };

    [[nodiscard]] std::string create_type_name(TypeTag single_tag);

    // [[nodiscard]] std::string create_type_name(const std::vector<TypeTag>& tupled_tags);

    // [[nodiscard]] std::string create_type_name(TypeTag item_tag, int count);
}