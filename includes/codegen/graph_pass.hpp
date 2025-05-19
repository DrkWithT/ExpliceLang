#pragma once

#include <any>
#include <queue>
#include <variant>
#include <vector>
#include "syntax/expr_visitor_base.hpp"
#include "syntax/stmt_visitor_base.hpp"
#include "syntax/stmts.hpp"
#include "codegen/flow_nodes.hpp"

namespace XLang::Codegen {
    class RegisterAllocator {
    public:
        RegisterAllocator(std::size_t register_n);

        [[nodiscard]] int allocate();
        [[nodiscard]] bool release(int reg);

    private:
        std::vector<bool> m_regs;
    };

    class OffsetAllocator {
    public:
        OffsetAllocator(std::size_t stack_capacity);

        [[nodiscard]] int allocate();
        [[nodiscard]] bool release(int slot);

    private:
        std::vector<bool> m_slots;
        std::vector<int> m_lru;

        [[nodiscard]] int salvage_from_lru() noexcept;
    };

    using ExprVisitResult = std::variant<int, float, Locator>;

    /**
     * @brief Builds a FlowGraph from an AST.
     */
    class GraphPass : public Syntax::ExprVisitor<ExprVisitResult>, public Syntax::StmtVisitor<std::any> {
    private:
        RegisterAllocator m_reg_all;
        OffsetAllocator m_off_all;

        /// @note refers new nodes to connect later
        std::queue<FlowGraph*> m_nodes;

        /// @note refers flow graph being built
        FlowStore* m_result;

        [[nodiscard]] Locator new_location();
        [[maybe_unused]] bool delete_location(const Locator& loc);

    public:
        GraphPass(std::size_t vm_regs_n, std::size_t vm_stack_n);

        [[nodiscard]] ExprVisitResult visit_literal(const Syntax::Literal& expr) override;
        [[nodiscard]] ExprVisitResult visit_unary(const Syntax::Unary& expr) override;
        [[nodiscard]] ExprVisitResult visit_binary(const Syntax::Binary& expr) override;
        [[nodiscard]] ExprVisitResult visit_call(const Syntax::Call& expr) override;

        [[nodiscard]] std::any visit_import(const Syntax::Import& stmt) override;
        [[nodiscard]] std::any visit_variable_decl(const Syntax::VariableDecl& stmt) override;
        [[nodiscard]] std::any visit_function_decl(const Syntax::FunctionDecl& stmt) override;
        [[nodiscard]] std::any visit_expr_stmt(const Syntax::ExprStmt& stmt) override;
        [[nodiscard]] std::any visit_block(const Syntax::Block& stmt) override;
        [[nodiscard]] std::any visit_return(const Syntax::Return& stmt) override;
        [[nodiscard]] std::any visit_if(const Syntax::If& stmt) override;

        [[nodiscard]] FlowStore* process(const std::vector<Syntax::StmtPtr>& ast);
    };
}