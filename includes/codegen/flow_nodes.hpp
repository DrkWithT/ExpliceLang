#pragma once

#include <unordered_map>
#include <vector>
#include <variant>
#include "codegen/steps.hpp"

namespace XLang::Codegen {
    struct Unit;
    struct Juncture;
    class FlowGraph;

    using NodeUnion = std::variant<Unit, Juncture>;
    using StepUnion = std::variant<NonaryStep, UnaryStep, BinaryStep, TernaryStep>;
    using StepSequence = std::vector<StepUnion>;
    using FlowStore = std::unordered_map<int, FlowGraph>;

    /// @note Represents up to two child ids per node.
    struct ChildPair {
        int first;
        int second;
    };

    /// @note Represents a non-forking sequence of instructions.
    struct Unit {
        StepSequence steps;
        int next;
    };

    /// @note Connects Unit nodes of the control-flow graph.
    struct Juncture {
        int left; // truthy branch
        int right; // falsy branch
    };

    /// @note Models control-flow per function.
    class FlowGraph {
    private:
        /// @note All nodes are stored in adjacency lists but vectors are cache-friendlier.
        std::vector<NodeUnion> m_items;

        [[nodiscard]] bool validate_id(int id) const noexcept;

    public:
        FlowGraph();

        const std::vector<NodeUnion>& view_nodes() const noexcept;
        NodeUnion& node_at(int id) & noexcept;
        const NodeUnion& node_at(int id) const& noexcept;

        template <typename NodeBoxType>
        [[maybe_unused]] constexpr int add_node(NodeBoxType&& node) {
            const int next_node_id = m_items.size();

            m_items.emplace_back(node);

            return next_node_id;
        }

        [[nodiscard]] ChildPair find_neighbors(int source_id);

        /// @note Only use for Unit nodes.
        [[maybe_unused]] bool connect_neighbor(int target_id, int next_id);

        /// @note Only use for Juncture nodes.
        [[maybe_unused]] bool connect_neighbor(int target_id, int left_id, int right_id);

        /// @note Use for passes which can mutate the CFG.
        template <template <typename, typename> typename Pass, typename Result, typename Policy>
        Result take_pass(Pass<Result, Policy>& pass) {
            return pass.process(*this);
        }

        /// @note Use for passes which don't mutate the CFG.
        template <template <typename, typename> typename Pass, typename Result, typename Policy>
        Result take_pass(Pass<Result, Policy>& pass) const& {
            return pass.process(*this);
        }
    };
}