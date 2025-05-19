#pragma once

#include <type_traits>
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
        int left;
        int right;
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
    public:
        template <bool IsConstSelf, typename T>
        struct rooty_t {
            using type = T&;
        };

        template <typename T>
        struct rooty_t <true, T> {
            using type = const T&;
        };

    private:
        /// @note All nodes are stored in adjacency lists but vectors are cache-friendlier.
        std::vector<NodeUnion> m_items;
        std::vector<std::vector<int>> m_links;

    public:
        FlowGraph();

        constexpr auto root(this auto&& self) noexcept -> rooty_t<std::is_const_v<std::remove_reference_t<decltype(self)>>, NodeUnion> {
            return self.m_items[0];
        }

        [[nodiscard]] constexpr int add_node(std::same_as<NodeUnion> auto&& node) {
            m_items.emplace_back(node);
            m_links.push_back({});
        }

        [[nodiscard]] ChildPair find_neighbors(int source_id);
        [[nodiscard]] bool connect_neighbor(int target_id, int other_id);

        template <template <typename, typename> typename Pass, typename Result, typename Policy>
        Result take_pass(Pass<Result, Policy>& pass) {
            return pass.process(*this);
        }
    };
}