#include <array>
#include <sstream>
#include <format>
#include "semantics/tags.hpp"

namespace XLang::Semantics {
    std::string_view create_type_name(TypeTag single_tag) {
        static constinit std::array<std::string_view, static_cast<std::size_t>(TypeTag::x_type_unknown) + 1> dirty_names = {
            "bool",
            "int",
            "float",
            "string",
            "unknown"
        }; // quick and dirty mapping from type tags to names for comparisons...

        return dirty_names[static_cast<int>(single_tag)].data();
    }

    [[nodiscard]] std::string create_type_name(const ArrayType& array_tag) {
        return std::format("array-{}-{}", create_type_name(array_tag.item_tag), array_tag.n);
    }

    [[nodiscard]] std::string create_type_name(const TupleType& tuple_tag) {
        static std::ostringstream sout;
        sout.str("");

        sout << "tuple";

        for (const auto& cell_tag : tuple_tag.item_tags) {
            sout << '-' << create_type_name(cell_tag);
        }

        return sout.str();
    }
}