#pragma once

#include <any>
#include <unordered_map>
#include <vector>
#include "syntax/expr_visitor_base.hpp"
#include "syntax/stmt_visitor_base.hpp"
#include "syntax/stmts.hpp"
#include "codegen/flow_nodes.hpp"

namespace XLang::Codegen {
    /// @todo Implement! 
    // class RegisterAllocator {};

    /// @todo Implement!
    // class OffsetAllocator {};

    /**
     * @brief Builds a FlowGraph from an AST.
     */
    class GraphPass : public Syntax::ExprVisitor<std::any>, public Syntax::StmtVisitor<std::any> {
    private:
        /// @todo See above classes.
        // RegisterAllocator m_reg_all;
        // OffsetAllocator m_off_all;
        FlowGraph* m_temp;
        FlowStore* m_result;

        [[nodiscard]] Locator new_location();
        [[nodiscard]] bool delete_location(const Locator& loc);

    public:
        GraphPass();

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

        [[nodiscard]] FlowStore* process(const std::vector<Syntax::StmtPtr>& ast);
    };
}