#pragma once

#include <array>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include "syntax/expr_visitor_base.hpp"
#include "syntax/exprs.hpp"
#include "syntax/stmt_visitor_base.hpp"
#include "syntax/stmts.hpp"
#include "semantics/tags.hpp"

namespace XLang::Semantics {
    struct SemanticEntry {
        TypeInfo type;
        ValuingTag value_group;
    };

    enum class ErrorSubject : unsigned char {
        undefined_name,
        invalid_type,
        invalid_operation_on_type,
        general,
    };

    struct SemanticDump {
        std::string message;
        ErrorSubject subject;
        int line;
    };

    using Scope = std::unordered_map<std::string_view, SemanticEntry>;
    using SemanticDiagnoses = std::vector<SemanticDump>;

    /// @brief Checks for any undefined names and does simple type checking.
    class SemanticsPass : public Syntax::ExprVisitor<std::any>, public Syntax::StmtVisitor<std::any> {
    public:
        SemanticsPass();

        [[nodiscard]] SemanticDiagnoses operator()(const std::vector<Syntax::StmtPtr>& ast_decls);

        std::any visit_literal(const Syntax::Literal& expr) override;
        std::any visit_unary(const Syntax::Unary& expr) override;
        std::any visit_binary(const Syntax::Binary& expr) override;
        std::any visit_call(const Syntax::Call& expr) override;

        std::any visit_import(const Syntax::Import& stmt) override;
        std::any visit_variable_decl(const Syntax::VariableDecl& stmt) override;
        std::any visit_function_decl(const Syntax::FunctionDecl& stmt) override;
        std::any visit_expr_stmt(const Syntax::ExprStmt& stmt) override;
        std::any visit_block(const Syntax::Block& stmt) override;
        std::any visit_return(const Syntax::Return& stmt) override;
        std::any visit_if(const Syntax::If& stmt) override;

    private:
        /// @note relates primitive type to supported operations in order to more concisely achieve type checking. The key types include bool, int, float, string which each map to all possible OpType values.
        static constexpr std::array<std::array<bool, static_cast<std::size_t>(OpTag::last)>, static_cast<std::size_t>(TypeTag::last)> cm_basic_type_ops = {
            std::array<bool, static_cast<std::size_t>(OpTag::last)> {true, true, false, false, false, false, false, false, true, true, false, false, true, true},
            {true, true, false, true, true, true, true, true, true, true, true, true, false, false},
            {true, true, false, true, true, true, true, true, true, true, true, true, false, false},
            {true, true, true, false, false, false, false, false, true, true, false, false, false, false},
            {false, false, false, false, false, false, false, false, false, false, false, false, false, false}
        };

        std::vector<Scope> m_scopes;

        void enter_scope();
        void leave_scope();

        [[nodiscard]] TypeInfo resolve_name_existence(std::string_view name);
        [[nodiscard]] TypeInfo resolve_type_from(std::string_view name);
        [[nodiscard]] bool check_type_operation(const TypeInfo& arg_typing);
        [[nodiscard]] bool check_type_operation(const TypeInfo& lhs_typing, const TypeInfo& rhs_typing);

        void record_diagnosis(std::string message, ErrorSubject subject, int line);
    };
}