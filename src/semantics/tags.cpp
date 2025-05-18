#include <array>
#include <string_view>
#include "semantics/tags.hpp"

namespace XLang::Semantics {
    std::string create_type_name(TypeTag single_tag) {
        static constinit std::array<std::string_view, static_cast<std::size_t>(TypeTag::x_type_unknown) + 1> dirty_names = {
            "bool",
            "int",
            "float",
            "string",
            "unknown"
        }; // quick and dirty mapping from type tags to names for comparisons...

        return dirty_names[static_cast<int>(single_tag)].data();
    }
}