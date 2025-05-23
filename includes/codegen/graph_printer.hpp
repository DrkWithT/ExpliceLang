#pragma once

#include <string_view>
#include "codegen/steps.hpp"
#include "codegen/flow_nodes.hpp"

namespace XLang::Codegen {
    template <typename... Types>
    struct vprinter_overloads : Types... {
        using Types::operator()...;
    };

    [[nodiscard]] std::string_view get_opcode_name(VM::Opcode opcode) noexcept;

    [[nodiscard]] std::string_view get_region_name(Region region) noexcept;

    class FlowGraphPrinter {
    public:
        FlowGraphPrinter();

        void operator()(const XLang::Codegen::FlowStore& all_flow_graphs);

    private:
        void print_node_box(const NodeUnion& node_box);

        void print_unit(const Unit& unit);
        void print_juncture(const Juncture& juncture);
    };
}