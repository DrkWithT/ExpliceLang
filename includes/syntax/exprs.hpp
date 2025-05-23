#pragma once

#include <memory>
#include <string>
#include <vector>
#include "frontend/token.hpp"
#include "semantics/tags.hpp"
#include "syntax/expr_visitor_base.hpp"
#include "syntax/expr_base.hpp"

namespace XLang::Syntax {
    using ExprPtr = std::unique_ptr<Expr>;

    /// @note Represents primitive typed literals only. List, Array, and Tuple will be added later.
    struct Literal : public Expr {
        Frontend::Token token;
        Semantics::TypeTag type;
        bool refers_callable;

        explicit Literal(const Frontend::Token& token_, Semantics::TypeTag type_, bool refers_callable_) noexcept;

        bool yields_value() const noexcept override;
        Semantics::ValuingTag value_group() const noexcept override;
        std::any type_tagging() const override;
        std::any accept_visitor(ExprVisitor<std::any>& visitor) const override;
    };

    struct Unary : public Expr {
        ExprPtr inner;
        Semantics::OpTag op;

        Unary(ExprPtr inner_, Semantics::OpTag op_) noexcept;

        bool yields_value() const noexcept override;
        Semantics::ValuingTag value_group() const noexcept override;
        std::any type_tagging() const override;
        std::any accept_visitor(ExprVisitor<std::any>& visitor) const override;
    };

    struct Binary : public Expr {
        ExprPtr left;
        ExprPtr right;
        Semantics::OpTag op;

        Binary(ExprPtr left_, ExprPtr right_, Semantics::OpTag op_) noexcept;

        bool yields_value() const noexcept override;
        Semantics::ValuingTag value_group() const noexcept override;
        std::any type_tagging() const override;
        std::any accept_visitor(ExprVisitor<std::any>& visitor) const override;
    };

    struct Call : public Expr {
        std::vector<ExprPtr> args;
        std::string func_name;

        Call(std::vector<ExprPtr> args_, std::string func_name_) noexcept;

        bool yields_value() const noexcept override;
        Semantics::ValuingTag value_group() const noexcept override;
        std::any type_tagging() const override;
        std::any accept_visitor(ExprVisitor<std::any>& visitor) const override;
    };
}