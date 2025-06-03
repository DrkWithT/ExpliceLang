#pragma once

#include <string_view>
#include "codegen/steps.hpp"
#include "codegen/flow_nodes.hpp"
#include "codegen/graph_pass.hpp"

namespace XLang::Codegen {
    [[nodiscard]] std::string_view get_opcode_name(VM::Opcode opcode) noexcept;

    [[nodiscard]] std::string_view get_region_name(Region region) noexcept;

    class IRPrinter {
    public:
        IRPrinter();

        void operator()(const XLang::Codegen::IRStore& all_ir);

    private:
        void print_node_box(const NodeUnion& node_box);

        void print_unit(const Unit& unit);
        void print_juncture(const Juncture& juncture);
    };
}