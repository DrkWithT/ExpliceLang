#include <utility>
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

    std::any Literal::type_tagging() const {
        return type;
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

    std::any Unary::type_tagging() const {
        return inner->type_tagging();
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

    std::any Binary::type_tagging() const {
        const auto left_type = left->type_tagging();

        if (left_type.type() == typeid(Semantics::TypeTag)) {
            if (const auto left_tag = std::any_cast<Semantics::TypeTag>(left_type); left_tag != Semantics::TypeTag::x_type_unknown) {
                return left_tag;
            }
        }

        const auto right_type = right->type_tagging();

        if (right_type.type() == typeid(Semantics::TypeTag)) {
            if (const auto right_tag = std::any_cast<Semantics::TypeTag>(right_type); right_tag != Semantics::TypeTag::x_type_unknown) {
                return right_tag;
            }
        }

        return Semantics::TypeTag::x_type_unknown;
    }

    std::any Binary::accept_visitor(ExprVisitor<std::any>& visitor) const {
        return visitor.visit_binary(*this);
    }


    Call::Call(std::vector<ExprPtr> args_, ExprPtr inner_) noexcept
    : args {std::move(args_)}, inner {std::move(inner_)} {}

    bool Call::yields_value() const noexcept {
        return true;
    }

    Semantics::ValuingTag Call::value_group() const noexcept {
        return Semantics::ValuingTag::x_unknown_value;
    }

    std::any Call::type_tagging() const {
        return Semantics::TypeTag::x_type_unknown;
    }

    std::any Call::accept_visitor(ExprVisitor<std::any>& visitor) const {
        return visitor.visit_call(*this);
    }
}