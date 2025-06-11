#include <utility>
#include <typeinfo>
#include "syntax/exprs.hpp"

namespace XLang::Syntax {
    Literal::Literal(const Frontend::Token& token_, Semantics::TypeTag type_, bool refers_callable_) noexcept
    : token {token_}, type {type_}, refers_callable {refers_callable_} {}

    bool Literal::yields_value() const noexcept {
        return true;
    }

    Semantics::ValuingTag Literal::value_group() const noexcept {
        return Semantics::ValuingTag::x_out_value;
    }

    Semantics::TypeInfo Literal::type_tagging() const {
        return Semantics::PrimitiveType {
            .item_tag = type,
            .readonly = true,
        };
    }

    ExprArity Literal::arity() const noexcept {
        return ExprArity::one;
    }

    std::any Literal::accept_visitor(ExprVisitor<std::any>& visitor) const {
        return visitor.visit_literal(*this);
    }


    Unary::Unary(ExprPtr inner_, Semantics::OpTag op_) noexcept
    : inner {std::move(inner_)}, op {op_} {}

    bool Unary::yields_value() const noexcept {
        return inner->yields_value();
    }

    Semantics::ValuingTag Unary::value_group() const noexcept {
        return Semantics::ValuingTag::x_unknown_value;
    }

    Semantics::TypeInfo Unary::type_tagging() const {
        return inner->type_tagging();
    }

    ExprArity Unary::arity() const noexcept {
        return ExprArity::one;
    }

    std::any Unary::accept_visitor(ExprVisitor<std::any>& visitor) const  {
        return visitor.visit_unary(*this);
    }


    Binary::Binary(ExprPtr left_, ExprPtr right_, Semantics::OpTag op_) noexcept
    : left {std::move(left_)}, right {std::move(right_)}, op {op_} {}

    bool Binary::yields_value() const noexcept {
        return left->yields_value() && right->yields_value();
    }

    Semantics::ValuingTag Binary::value_group() const noexcept {
        if (op == Semantics::OpTag::access) {
            return Semantics::ValuingTag::x_in_value;
        }

        return Semantics::ValuingTag::x_out_value;
    }

    Semantics::TypeInfo Binary::type_tagging() const {
        const auto& left_type = left->type_tagging();

        if (!std::holds_alternative<Semantics::NullType>(left_type)) {
            return left_type;
        }

        const auto& right_type = right->type_tagging();

        if (!std::holds_alternative<Semantics::NullType>(right_type)) {
            return right_type;
        }

        return Semantics::NullType {};
    }

    ExprArity Binary::arity() const noexcept {
        return ExprArity::two;
    }

    std::any Binary::accept_visitor(ExprVisitor<std::any>& visitor) const {
        return visitor.visit_binary(*this);
    }


    Call::Call(std::vector<ExprPtr> args_, std::string func_name_) noexcept
    : args {std::move(args_)}, func_name {std::move(func_name_)} {}

    bool Call::yields_value() const noexcept {
        return true;
    }

    Semantics::ValuingTag Call::value_group() const noexcept {
        return Semantics::ValuingTag::x_unknown_value;
    }

    Semantics::TypeInfo Call::type_tagging() const {
        return Semantics::NullType {};
    }

    ExprArity Call::arity() const noexcept {
        return ExprArity::variadic;
    }

    std::any Call::accept_visitor(ExprVisitor<std::any>& visitor) const {
        return visitor.visit_call(*this);
    }

}