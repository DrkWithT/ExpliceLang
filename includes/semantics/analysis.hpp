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
    [[nodiscard]] TypeTag unpack_underlying_type_tag(const TypeInfo& info);

    [[nodiscard]] std::string_view op_tag_to_name(OpTag tag) noexcept;

    [[nodiscard]] std::string type_info_to_str(const TypeInfo& typing);

    [[nodiscard]] bool compare_type_info(const TypeInfo& lhs_type, const TypeInfo& rhs_type);

    struct SemanticEntry {
        TypeInfo type;
        ValuingTag value_group;
    };

    struct SemanticNativeEntry {
        TypeInfo signature_type;
        int id;
    };

    struct SemanticLocation {
        std::string_view name;
        int line;
    };

    struct SemanticDump {
        std::string message;
        Frontend::Token culprit;

        friend std::string stringify_sema_dump(const SemanticDump& dump);
    };

    struct SemanticResult {
        std::unordered_map<std::string_view, SemanticNativeEntry> native_hints;
        std::vector<SemanticDump> errors;
    };

    using Scope = std::unordered_map<std::string_view, SemanticEntry>;
    using NativeHints = std::unordered_map<std::string_view, SemanticNativeEntry>;
    using SemanticDiagnoses = std::vector<SemanticDump>;

    /// @brief Checks for any undefined names and does simple type checking.
    class SemanticsPass : public Syntax::ExprVisitor<std::any>, public Syntax::StmtVisitor<std::any> {
    public:
        SemanticsPass(std::string_view source_);

        [[nodiscard]] SemanticResult operator()(const std::vector<Syntax::StmtPtr>& ast_decls);

        std::any visit_literal(const Syntax::Literal& expr) override;
        std::any visit_unary(const Syntax::Unary& expr) override;
        std::any visit_binary(const Syntax::Binary& expr) override;
        std::any visit_call(const Syntax::Call& expr) override;

        std::any visit_native_use(const Syntax::NativeUse& stmt) override;
        std::any visit_import(const Syntax::Import& stmt) override;
        std::any visit_variable_decl(const Syntax::VariableDecl& stmt) override;
        std::any visit_function_decl(const Syntax::FunctionDecl& stmt) override;
        std::any visit_expr_stmt(const Syntax::ExprStmt& stmt) override;
        std::any visit_block(const Syntax::Block& stmt) override;
        std::any visit_return(const Syntax::Return& stmt) override;
        std::any visit_if(const Syntax::If& stmt) override;
        std::any visit_while(const Syntax::While& stmt) override;

    private:
        struct OpTypeCheckResult {
            TypeInfo result_data_type;
            bool ok;
        };

        /// @note relates primitive type to supported operations in order to more concisely achieve type checking. The key types include bool, int, float, string which each map to all possible OpType values.
        static constexpr std::array<std::array<bool, static_cast<std::size_t>(OpTag::last)>, static_cast<std::size_t>(TypeTag::last)> cm_basic_type_ops = {
            std::array<bool, static_cast<std::size_t>(OpTag::last)> {true, true, false, false, false, false, false, false, true, true, false, false, true, true},
            {true, true, false, true, true, true, true, true, true, true, true, true, false, false},
            {true, true, false, true, true, true, true, true, true, true, true, true, false, false},
            {true, true, true, false, false, false, false, false, true, true, false, false, false, false},
            {false, false, false, false, false, false, false, false, false, false, false, false, false, false}
        };

        NativeHints m_native_hints;
        std::vector<Scope> m_scopes;
        SemanticLocation m_location;
        SemanticDiagnoses m_result;
        std::string_view m_source;

        void enter_scope();
        void leave_scope();

        void record_proc_name(std::string_view name, TypeInfo info);
        void record_name(std::string_view name, TypeInfo info);
        [[nodiscard]] bool resolve_name_existence(std::string_view name);

        void record_native_name(std::string_view name, TypeInfo info);

        /// @note If the name is undeclared, the TypeInfo contains a NullType.
        [[nodiscard]] TypeInfo resolve_type_from(std::string_view name);
        [[nodiscard]] OpTypeCheckResult check_type_operation(OpTag op, const TypeInfo& arg_typing);
        [[nodiscard]] OpTypeCheckResult check_type_operation(OpTag op, const TypeInfo& lhs_typing, const TypeInfo& rhs_typing);

        void record_diagnosis(std::string message, Frontend::Token culprit);
    };
}