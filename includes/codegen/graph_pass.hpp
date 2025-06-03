#pragma once

#include <memory>
#include <any>
#include <unordered_map>
#include <variant>
#include <vector>
#include "semantics/tags.hpp"
#include "syntax/expr_visitor_base.hpp"
#include "syntax/stmt_visitor_base.hpp"
#include "syntax/stmts.hpp"
#include "codegen/flow_nodes.hpp"

namespace XLang::Codegen {
    struct ConstPrimitiveInfo;
    struct IRStore;

    using ProtoConstMap = std::unordered_map<std::string_view, ConstPrimitiveInfo>;
    using HeapObjectInfo = std::variant<Semantics::NullType, Semantics::ArrayType, Semantics::TupleType>;
    using NameLocatorRecord = std::unordered_map<std::string_view, Locator>;

    struct ConstPrimitiveInfo {
        std::variant<bool, int, float> data;
        int id;
    };

    struct IRStore {
        std::vector<ProtoConstMap> const_chunks;
        std::unique_ptr<FlowStore> func_cfgs;
        int main_func_id;
    };

    class HeapAllocator {
    public:
        HeapAllocator();

        [[nodiscard]] const HeapObjectInfo& lookup_info(int id) const noexcept;
        [[nodiscard]] int allocate(Semantics::ArrayType array_tag);
        [[nodiscard]] int allocate(Semantics::TupleType tuple_tag);
        [[nodiscard]] bool release(int id);

    private:
        std::unordered_map<int, HeapObjectInfo> m_items;
        std::vector<int> m_free_list;

        [[nodiscard]] static int next_id() noexcept;

        [[nodiscard]] int salvage_free_gap_id();
    };

    /**
     * @brief Builds a FlowGraph from an AST.
     */
    class GraphPass : public Syntax::ExprVisitor<std::any>, public Syntax::StmtVisitor<std::any> {
    private:
        enum class OpLeaning {
            lean_left,
            lean_right,
        };

        HeapAllocator m_heap_all;
        NameLocatorRecord m_current_name_map;
        NameLocatorRecord m_current_params_map;
        NameLocatorRecord m_global_func_map;
        ProtoConstMap m_const_map;

        /// @note Stores compiled constant primitives per function chunk.
        std::vector<ProtoConstMap> m_func_consts;

        /// @note refers new nodes to connect later
        std::vector<NodeUnion> m_nodes;

        /// @note refers current building flow graph
        std::unique_ptr<FlowGraph> m_graph;

        /// @note holds result pointer to return
        std::unique_ptr<FlowStore> m_result;

        std::string_view m_old_src;

        int m_main_func_idx;

        /// @note const_id resets after each function decl processed... const_id = m_const_map.size()
        [[nodiscard]] int next_const_id() noexcept;
        [[nodiscard]] int next_local_id() noexcept;
        [[nodiscard]] int next_func_id() noexcept;
        [[nodiscard]] int next_param_id() noexcept;
        [[nodiscard]] int curr_const_id() noexcept;
        [[nodiscard]] int curr_local_id() noexcept;
        [[nodiscard]] int curr_func_id() noexcept;
        [[nodiscard]] int curr_param_id() noexcept;
        [[nodiscard]] Locator new_obj_location(Semantics::ArrayType array_tag);
        [[nodiscard]] Locator new_obj_location(Semantics::TupleType tuple_tag);
        [[maybe_unused]] bool delete_location(const Locator& loc);
        const Locator& lookup_named_location(std::string_view name) const;
        const Locator& lookup_callable_name(std::string_view name) const;

        void commit_current_consts();

        void leave_record();
        void place_step(StepUnion step);
        void place_node(NodeUnion node_box);

        /// @note Puts all queued node boxes into the currently referenced `FlowGraph`, connected, before moving the graph into the referenced `FlowStore`. Assumes the current graph is initially EMPTY and function decls. are processed TOP-TO-BOTTOM!
        void commit_nodes_to_graph(int current_func_id, bool all_decls_done);

        [[nodiscard]] std::any help_gen_access(const Syntax::Binary& expr);
        [[nodiscard]] std::any help_gen_arithmetic(OpLeaning op_lean, const Syntax::Binary& expr);
        [[nodiscard]] std::any help_gen_compare(OpLeaning op_lean, const Syntax::Binary& expr);
        [[nodiscard]] std::any help_gen_logical(const Syntax::Binary& expr);
        [[nodiscard]] std::any help_gen_assign(const Syntax::Binary& expr);

    public:
        GraphPass(std::string_view old_source);

        [[nodiscard]] std::any visit_literal(const Syntax::Literal& expr) override;
        [[nodiscard]] std::any visit_unary(const Syntax::Unary& expr) override;
        [[nodiscard]] std::any visit_binary(const Syntax::Binary& expr) override;
        [[nodiscard]] std::any visit_call(const Syntax::Call& expr) override;

        [[nodiscard]] std::any visit_import(const Syntax::Import& stmt) override;
        [[nodiscard]] std::any visit_variable_decl(const Syntax::VariableDecl& stmt) override;
        [[nodiscard]] std::any visit_function_decl(const Syntax::FunctionDecl& stmt) override;
        [[nodiscard]] std::any visit_expr_stmt(const Syntax::ExprStmt& stmt) override;
        [[nodiscard]] std::any visit_block(const Syntax::Block& stmt) override;
        [[nodiscard]] std::any visit_return(const Syntax::Return& stmt) override;
        [[nodiscard]] std::any visit_if(const Syntax::If& stmt) override;

        [[nodiscard]] IRStore process(const std::vector<Syntax::StmtPtr>& ast);
    };
}