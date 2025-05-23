#include <array>
#include <string_view>
#include <print>
#include "codegen/graph_printer.hpp"

namespace XLang::Codegen {
    static constexpr std::array<std::string_view, static_cast<std::size_t>(VM::Opcode::last)> opcode_names = {
        "halt",
        "noop",
        "push",
        "pop",
        "peek",
        "load_const",
        "make_array",
        "make_tuple",
        "access_field",
        "negate",
        "add",
        "sub",
        "mul",
        "div",
        "cmp_eq",
        "cmp_ne",
        "cmp_lt",
        "cmp_gt",
        "log_and",
        "log_or",
        "jump",
        "jump_if",
        "enter",
        "ret",
        "call",
        "call_native"
    };

    static constexpr std::array<std::string_view, static_cast<std::size_t>(Region::last)> region_names = {
        "consts",
        "stack",
        "heap",
        "routines",
        "none"
    };

    std::string_view get_opcode_name(VM::Opcode opcode) noexcept {
        return opcode_names[static_cast<int>(opcode)];
    }

    std::string_view get_region_name(Region region) noexcept {
        return region_names[static_cast<int>(region)];
    }

    FlowGraphPrinter::FlowGraphPrinter() {}

    void FlowGraphPrinter::operator()(const XLang::Codegen::FlowStore& all_flow_graphs) {
        for (const auto& [func_id, func_flows] : all_flow_graphs) {
            std::print("Function chunk {}:\n\n", func_id);
            print_node_box(func_flows.node_at(0));
        }
    }

    void FlowGraphPrinter::print_node_box(const NodeUnion& node_box) {
        if (node_box.index() == 0) {
            print_unit(std::get<Unit>(node_box));
        } else {
            print_juncture(std::get<Juncture>(node_box));
        }
    }

    void FlowGraphPrinter::print_unit(const Unit& unit) {
        std::print("Unit:\n.next = {}\n\n", unit.next);
        for (const auto& step : unit.steps) {
            if (step.index() == 0) {
                const auto& step_nonary = std::get<NonaryStep>(step);
                std::print("{}\n", get_opcode_name(step_nonary.op));
            } else if (step.index() == 1) {
                const auto& step_unary = std::get<UnaryStep>(step);
                const auto& [region, id] = step_unary.arg_0;
                std::print("{} {}:{}\n", get_opcode_name(step_unary.op), get_region_name(region), id);
            } else if (step.index() == 2) {
                const auto& step_binary = std::get<BinaryStep>(step);
                const auto& [region, id] = step_binary.arg_0;
                const auto& [region2, id2] = step_binary.arg_1;

                std::print("{} {}:{}, {}:{}\n", get_opcode_name(step_binary.op), get_region_name(region), id, get_region_name(region2), id2);
            } else if (step.index() == 3) {
                const auto& step_ternary = std::get<TernaryStep>(step);

                const auto& [region, id] = step_ternary.arg_0;
                const auto& [region2, id2] = step_ternary.arg_1;
                const auto& [region3, id3] = step_ternary.arg_2;

                std::print("{} {}:{}, {}:{}, {}:{}\n", get_opcode_name(step_ternary.op), get_region_name(region), id, get_region_name(region2), id2, get_region_name(region3), id3);
            } else {}
        }
    }

    void FlowGraphPrinter::print_juncture(const Juncture& juncture) {
        std::print("Juncture:\n\t.left = {}, .right = {}\n\n", juncture.left, juncture.right);
    }
}