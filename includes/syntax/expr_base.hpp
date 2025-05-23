#pragma once

#include <any>
#include "semantics/tags.hpp"
#include "syntax/expr_visitor_base.hpp"

namespace XLang::Syntax {
    struct Expr {
        virtual ~Expr() = default;

        virtual bool yields_value() const noexcept = 0;
        virtual Semantics::ValuingTag value_group() const noexcept = 0;
        virtual std::any type_tagging() const = 0;
        virtual std::any accept_visitor(ExprVisitor<std::any>& visitor) const = 0;
    };
}