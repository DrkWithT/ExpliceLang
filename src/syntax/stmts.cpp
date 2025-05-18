#include <utility>
#include "syntax/stmts.hpp"

namespace XLang::Syntax {
    Import::Import(const Frontend::Token& unit_name_) noexcept
    : unit_name {unit_name_} {}

    bool Import::is_declarative() const noexcept {
        return true;
    }

    bool Import::is_control_flow() const noexcept {
        return false;
    }

    bool Import::is_expr_stmt() const noexcept {
        return false;
    }

    std::any Import::possible_result_type() const noexcept {
        return {};
    }

    void Import::accept_visitor(StmtVisitor<void>& visitor) const {
        visitor.visit_import(*this);
    }


    VariableDecl::VariableDecl(std::any typing_, const Frontend::Token& name_, ExprPtr init_expr_, bool readonly_) noexcept
    : typing {typing_}, name {name_}, init_expr {std::move(init_expr_)}, readonly {readonly_} {}

    bool VariableDecl::is_declarative() const noexcept {
        return true;
    }

    bool VariableDecl::is_control_flow() const noexcept {
        return false;
    }

    bool VariableDecl::is_expr_stmt() const noexcept {
        return false;
    }

    std::any VariableDecl::possible_result_type() const noexcept {
        return typing;
    }

    void VariableDecl::accept_visitor(StmtVisitor<void>& visitor) const {
        visitor.visit_variable_decl(*this);
    }


    FunctionDecl::FunctionDecl(std::any typing_, const std::vector<ArgDecl>& args_, const Frontend::Token& name_, StmtPtr body_) noexcept
    : typing {typing_}, args {args_}, name {name_}, body {std::move(body_)} {}

    bool FunctionDecl::is_declarative() const noexcept {
        return true;
    }

    bool FunctionDecl::is_control_flow() const noexcept {
        return false;
    }

    bool FunctionDecl::is_expr_stmt() const noexcept {
        return false;
    }

    std::any FunctionDecl::possible_result_type() const noexcept {
        return typing;
    }

    void FunctionDecl::accept_visitor(StmtVisitor<void>& visitor) const {
        visitor.visit_function_decl(*this);
    }


    ExprStmt::ExprStmt(ExprPtr inner_) noexcept
    : inner {std::move(inner_)} {}

    bool ExprStmt::is_declarative() const noexcept {
        return false;
    }

    bool ExprStmt::is_control_flow() const noexcept {
        return false;
    }

    bool ExprStmt::is_expr_stmt() const noexcept {
        return true;
    }

    std::any ExprStmt::possible_result_type() const noexcept {
        return (inner->yields_value())
        ? inner->type_tagging()
        : Semantics::TypeTag::x_type_unknown;
    }

    void ExprStmt::accept_visitor(StmtVisitor<void>& visitor) const {
        visitor.visit_expr_stmt(*this);
    }


    Block::Block(std::vector<StmtPtr> stmts_) noexcept
    : stmts {std::move(stmts_)} {}

    bool Block::is_declarative() const noexcept {
        return false;
    }

    bool Block::is_control_flow() const noexcept {
        return true;
    }

    bool Block::is_expr_stmt() const noexcept {
        return false;
    }

    std::any Block::possible_result_type() const noexcept {
        return {};
    }

    void Block::accept_visitor(StmtVisitor<void>& visitor) const {
        visitor.visit_block(*this);
    }


    Return::Return(ExprPtr result_expr_) noexcept
    : result_expr {std::move(result_expr_)} {}

    bool Return::is_declarative() const noexcept {
        return false;
    }

    bool Return::is_control_flow() const noexcept {
        return true;
    }

    bool Return::is_expr_stmt() const noexcept {
        return false;
    }

    std::any Return::possible_result_type() const noexcept {
        return (result_expr->yields_value())
            ? result_expr->type_tagging()
            : Semantics::TypeTag::x_type_unknown;
    }

    void Return::accept_visitor(StmtVisitor<void>& visitor) const {
        visitor.visit_return(*this);
    }


    If::If(ExprPtr test_, StmtPtr truthy_body_, StmtPtr falsy_body_) noexcept
    : test {std::move(test_)}, truthy_body {std::move(truthy_body_)}, falsy_body {std::move(falsy_body_)} {};

    bool If::is_declarative() const noexcept {
        return false;
    }

    bool If::is_control_flow() const noexcept {
        return true;
    }

    bool If::is_expr_stmt() const noexcept {
        return false;
    }

    std::any If::possible_result_type() const noexcept {
        return {};
    }

    void If::accept_visitor(StmtVisitor<void>& visitor) const {
        visitor.visit_if(*this);
    }
}