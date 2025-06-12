#pragma once

#include <any>
#include "semantics/tags.hpp"
#include "syntax/stmt_visitor_base.hpp"

namespace XLang::Syntax {
    struct Stmt {
        virtual ~Stmt() = default;

        virtual bool is_directive() const noexcept = 0;
        virtual bool is_declarative() const noexcept = 0;
        virtual bool is_control_flow() const noexcept = 0;
        virtual bool is_expr_stmt() const noexcept = 0;
        virtual Semantics::TypeInfo possible_result_type() const noexcept = 0;
        virtual void accept_visitor(StmtVisitor<void>& visitor) const = 0;
        virtual std::any accept_visitor(StmtVisitor<std::any>& visitor) const = 0;
    };
}