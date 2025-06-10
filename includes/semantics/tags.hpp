#pragma once

#include <any>
#include <variant>
#include <vector>
#include <string>
#include <string_view>

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
        logic_or,
        last
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
        x_type_unknown,
        last
    };

    struct NullType {};

    struct PrimitiveType {
        TypeTag item_tag;
        bool readonly;
    };

    struct ArrayType {
        TypeTag item_tag;
        int n;
    };

    struct TupleType {
        std::vector<TypeTag> item_tags;
    };

    struct CallableType;

    struct CallableType {
        std::vector<std::variant<NullType, PrimitiveType, ArrayType, TupleType, CallableType>> item_tags;
        std::any result_tag; // @note Use std::any here because CallableType is incomplete here.
    };

    using TypeInfo = std::variant<NullType, PrimitiveType, ArrayType, TupleType, CallableType>;

    [[nodiscard]] std::string_view create_type_name(TypeTag single_tag);

    [[nodiscard]] std::string create_type_name(const ArrayType& array_tag);

    [[nodiscard]] std::string create_type_name(const TupleType& tuple_tag);
}