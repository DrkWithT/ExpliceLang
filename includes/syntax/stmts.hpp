#pragma once

#include <memory>
#include <vector>
#include "frontend/token.hpp"
#include "syntax/exprs.hpp"
#include "syntax/stmt_visitor_base.hpp"
#include "syntax/stmt_base.hpp"

namespace XLang::Syntax {
    using StmtPtr = std::unique_ptr<Stmt>;

    struct ArgDecl {
        Semantics::TypeInfo type;
        Frontend::Token name;
    };

    /// @brief Represents `<use-native>` statements within Xplice code so that native functions are both forward declared and activated.
    struct NativeUse : public Stmt {
        Semantics::TypeInfo typing;
        std::vector<ArgDecl> args;
        Frontend::Token native_name;

        NativeUse(Semantics::TypeInfo typing_, std::vector<ArgDecl> args_, Frontend::Token native_name_) noexcept;

        bool is_directive() const noexcept override;
        bool is_declarative() const noexcept override;
        bool is_control_flow() const noexcept override;
        bool is_expr_stmt() const noexcept override;
        Semantics::TypeInfo possible_result_type() const noexcept override;
        void accept_visitor(StmtVisitor<void>& visitor) const override;
        std::any accept_visitor(StmtVisitor<std::any>& visitor) const override;
    };

    struct Import : public Stmt {
        Frontend::Token unit_name;

        Import(const Frontend::Token& unit_name_) noexcept;

        bool is_directive() const noexcept override;
        bool is_declarative() const noexcept override;
        bool is_control_flow() const noexcept override;
        bool is_expr_stmt() const noexcept override;
        Semantics::TypeInfo possible_result_type() const noexcept override;
        void accept_visitor(StmtVisitor<void>& visitor) const override;
        std::any accept_visitor(StmtVisitor<std::any>& visitor) const override;
    };

    struct VariableDecl : public Stmt {
        Semantics::TypeInfo typing;
        Frontend::Token name;
        ExprPtr init_expr;
        bool readonly;

        explicit VariableDecl(Semantics::TypeInfo typing_, const Frontend::Token& var_name_, ExprPtr init_expr_, bool readonly_) noexcept;

        bool is_directive() const noexcept override;
        bool is_declarative() const noexcept override;
        bool is_control_flow() const noexcept override;
        bool is_expr_stmt() const noexcept override;
        Semantics::TypeInfo possible_result_type() const noexcept override;
        void accept_visitor(StmtVisitor<void>& visitor) const override;
        std::any accept_visitor(StmtVisitor<std::any>& visitor) const override;
    };

    struct FunctionDecl : public Stmt {
        Semantics::TypeInfo typing;
        std::vector<ArgDecl> args;
        Frontend::Token name;
        StmtPtr body;

        FunctionDecl(Semantics::TypeInfo typing_, const std::vector<ArgDecl>& args_, const Frontend::Token& name_, StmtPtr body_) noexcept;

        bool is_directive() const noexcept override;
        bool is_declarative() const noexcept override;
        bool is_control_flow() const noexcept override;
        bool is_expr_stmt() const noexcept override;
        Semantics::TypeInfo possible_result_type() const noexcept override;
        void accept_visitor(StmtVisitor<void>& visitor) const override;
        std::any accept_visitor(StmtVisitor<std::any>& visitor) const override;
    };

    struct ExprStmt : public Stmt {
        ExprPtr inner;

        ExprStmt(ExprPtr inner_) noexcept;

        bool is_directive() const noexcept override;
        bool is_declarative() const noexcept override;
        bool is_control_flow() const noexcept override;
        bool is_expr_stmt() const noexcept override;
        Semantics::TypeInfo possible_result_type() const noexcept override;
        void accept_visitor(StmtVisitor<void>& visitor) const override;
        std::any accept_visitor(StmtVisitor<std::any>& visitor) const override;
    };

    struct Block : public Stmt {
        std::vector<StmtPtr> stmts;

        Block(std::vector<StmtPtr> stmts_) noexcept;

        bool is_directive() const noexcept override;
        bool is_declarative() const noexcept override;
        bool is_control_flow() const noexcept override;
        bool is_expr_stmt() const noexcept override;
        Semantics::TypeInfo possible_result_type() const noexcept override;
        void accept_visitor(StmtVisitor<void>& visitor) const override;
        std::any accept_visitor(StmtVisitor<std::any>& visitor) const override;
    };

    struct Return : public Stmt {
        ExprPtr result_expr;

        Return(ExprPtr result_expr_) noexcept;

        bool is_directive() const noexcept override;
        bool is_declarative() const noexcept override;
        bool is_control_flow() const noexcept override;
        bool is_expr_stmt() const noexcept override;
        Semantics::TypeInfo possible_result_type() const noexcept override;
        void accept_visitor(StmtVisitor<void>& visitor) const override;
        std::any accept_visitor(StmtVisitor<std::any>& visitor) const override;
    };

    struct If : public Stmt {
        ExprPtr test;
        StmtPtr truthy_body;
        StmtPtr falsy_body;

        If(ExprPtr test_, StmtPtr truthy_body_, StmtPtr falsy_body_) noexcept;

        bool is_directive() const noexcept override;
        bool is_declarative() const noexcept override;
        bool is_control_flow() const noexcept override;
        bool is_expr_stmt() const noexcept override;
        Semantics::TypeInfo possible_result_type() const noexcept override;
        void accept_visitor(StmtVisitor<void>& visitor) const override;
        std::any accept_visitor(StmtVisitor<std::any>& visitor) const override;
    };
}