#pragma once

#include <memory>
#include <vector>
#include "frontend/token.hpp"
#include "syntax/exprs.hpp"
#include "syntax/stmt_visitor_base.hpp"
#include "syntax/stmt_base.hpp"

namespace XLang::Syntax {
    using StmtPtr = std::unique_ptr<Stmt>;
    
    struct Import : public Stmt {
        Frontend::Token unit_name;

        Import(const Frontend::Token& unit_name_) noexcept;

        bool is_declarative() const noexcept override;
        bool is_control_flow() const noexcept override;
        bool is_expr_stmt() const noexcept override;
        std::any possible_result_type() const noexcept override;
        void accept_visitor(StmtVisitor<void>& visitor) const override;
    };

    struct VariableDecl : public Stmt {
        std::any typing;
        Frontend::Token name;
        ExprPtr init_expr;
        bool readonly;

        explicit VariableDecl(std::any typing_, const Frontend::Token& var_name_, ExprPtr init_expr_, bool readonly_) noexcept;

        bool is_declarative() const noexcept override;
        bool is_control_flow() const noexcept override;
        bool is_expr_stmt() const noexcept override;
        std::any possible_result_type() const noexcept override;
        void accept_visitor(StmtVisitor<void>& visitor) const override;
    };

    struct ArgDecl {
        std::any type;
        Frontend::Token name;
    };

    struct FunctionDecl : public Stmt {
        std::any typing;
        std::vector<ArgDecl> args;
        Frontend::Token name;
        StmtPtr body;

        FunctionDecl(std::any typing_, const std::vector<ArgDecl>& args_, const Frontend::Token& name_, StmtPtr body_) noexcept;

        bool is_declarative() const noexcept override;
        bool is_control_flow() const noexcept override;
        bool is_expr_stmt() const noexcept override;
        std::any possible_result_type() const noexcept override;
        void accept_visitor(StmtVisitor<void>& visitor) const override;
    };

    struct ExprStmt : public Stmt {
        ExprPtr inner;

        ExprStmt(ExprPtr inner_) noexcept;

        bool is_declarative() const noexcept override;
        bool is_control_flow() const noexcept override;
        bool is_expr_stmt() const noexcept override;
        std::any possible_result_type() const noexcept override;
        void accept_visitor(StmtVisitor<void>& visitor) const override;
    };

    struct Block : public Stmt {
        std::vector<StmtPtr> stmts;

        Block(std::vector<StmtPtr> stmts_) noexcept;

        bool is_declarative() const noexcept override;
        bool is_control_flow() const noexcept override;
        bool is_expr_stmt() const noexcept override;
        std::any possible_result_type() const noexcept override;
        void accept_visitor(StmtVisitor<void>& visitor) const override;
    };

    struct Return : public Stmt {
        ExprPtr result_expr;

        Return(ExprPtr result_expr_) noexcept;

        bool is_declarative() const noexcept override;
        bool is_control_flow() const noexcept override;
        bool is_expr_stmt() const noexcept override;
        std::any possible_result_type() const noexcept override;
        void accept_visitor(StmtVisitor<void>& visitor) const override;
    };

    struct If : public Stmt {
        ExprPtr test;
        StmtPtr truthy_body;
        StmtPtr falsy_body;

        If(ExprPtr test_, StmtPtr truthy_body_, StmtPtr falsy_body_) noexcept;

        bool is_declarative() const noexcept override;
        bool is_control_flow() const noexcept override;
        bool is_expr_stmt() const noexcept override;
        std::any possible_result_type() const noexcept override;
        void accept_visitor(StmtVisitor<void>& visitor) const override;
    };
}