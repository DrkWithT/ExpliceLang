#include <algorithm>
#include "codegen/flow_nodes.hpp"

namespace XLang::Codegen {
    constexpr auto dud_node_id = -1;
    constexpr ChildPair dud_pair {-1, -1};
    constexpr auto unit_index = 0UL;
    constexpr auto juncture_index = 1UL;

    bool FlowGraph::validate_id(int id) const noexcept {
        return id >= 0 && id < static_cast<int>(m_items.size());
    }

    FlowGraph::FlowGraph()
    : m_items {} {}

    const std::vector<NodeUnion>& FlowGraph::view_nodes() const noexcept {
        return m_items;
    }

    NodeUnion& FlowGraph::node_at(int id) & noexcept {
        return m_items[id];
    }

    const NodeUnion& FlowGraph::node_at(int id) const& noexcept {
        return m_items[id];
    }

    ChildPair FlowGraph::find_neighbors(int source_id) {
        if (!validate_id(source_id)) {
            return dud_pair;
        }

        const auto& target_box = m_items[source_id];

        if (target_box.index() == unit_index) {
            const auto& unit_ref = std::get<Unit>(target_box);

            return {
                .first = unit_ref.next,
                .second = dud_node_id
            };
        } else if (target_box.index() == juncture_index) {
            const auto& [juncture_left, juncture_right] = std::get<Juncture>(target_box);

            return {
                .first = juncture_left,
                .second = juncture_right
            };
        }

        return dud_pair;
    }

    bool FlowGraph::connect_neighbor(int target_id, int next_id) {
        if (!validate_id(target_id) || !validate_id(next_id)) {
            return false;
        }

        auto& target_node = std::get<Unit>(m_items[target_id]);

        target_node.next = next_id;

        return true;
    }

    bool FlowGraph::connect_neighbor(int target_id, int left_id, int right_id) {
        if (!validate_id(target_id) || !validate_id(left_id) || !validate_id(right_id)) {
            return false;
        }

        auto& target_node = std::get<Juncture>(m_items[target_id]);

        target_node.left = left_id;
        target_node.right = right_id;

        return true;
    }
}