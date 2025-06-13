#include <utility>
#include "syntax/stmts.hpp"

namespace XLang::Syntax {
    NativeUse::NativeUse(Semantics::TypeInfo typing_, std::vector<ArgDecl> args_, Frontend::Token native_name_) noexcept
    : typing {std::move(typing_)}, args {std::move(args_)}, native_name {native_name_} {}

    bool NativeUse::is_directive() const noexcept {
        return true;
    }

    bool NativeUse::is_declarative() const noexcept {
        return true;
    }

    bool NativeUse::is_control_flow() const noexcept {
        return false;
    }

    bool NativeUse::is_expr_stmt() const noexcept {
        return false;
    }

    Semantics::TypeInfo NativeUse::possible_result_type() const noexcept {
        return typing;
    }

    void NativeUse::accept_visitor(StmtVisitor<void>& visitor) const {
        visitor.visit_native_use(*this);
    }

    std::any NativeUse::accept_visitor(StmtVisitor<std::any>& visitor) const {
        return visitor.visit_native_use(*this);
    }


    Import::Import(const Frontend::Token& unit_name_) noexcept
    : unit_name {unit_name_} {}

    bool Import::is_directive() const noexcept {
        return true;
    }

    bool Import::is_declarative() const noexcept {
        return true;
    }

    bool Import::is_control_flow() const noexcept {
        return false;
    }

    bool Import::is_expr_stmt() const noexcept {
        return false;
    }

    Semantics::TypeInfo Import::possible_result_type() const noexcept {
        return {};
    }

    void Import::accept_visitor(StmtVisitor<void>& visitor) const {
        visitor.visit_import(*this);
    }

    std::any Import::accept_visitor(StmtVisitor<std::any>& visitor) const {
        return visitor.visit_import(*this);
    }


    VariableDecl::VariableDecl(Semantics::TypeInfo typing_, const Frontend::Token& name_, ExprPtr init_expr_, bool readonly_) noexcept
    : typing {std::move(typing_)}, name {name_}, init_expr {std::move(init_expr_)}, readonly {readonly_} {}

    bool VariableDecl::is_directive() const noexcept {
        return false;
    }

    bool VariableDecl::is_declarative() const noexcept {
        return true;
    }

    bool VariableDecl::is_control_flow() const noexcept {
        return false;
    }

    bool VariableDecl::is_expr_stmt() const noexcept {
        return false;
    }

    Semantics::TypeInfo VariableDecl::possible_result_type() const noexcept {
        return typing;
    }

    void VariableDecl::accept_visitor(StmtVisitor<void>& visitor) const {
        visitor.visit_variable_decl(*this);
    }

    std::any VariableDecl::accept_visitor(StmtVisitor<std::any>& visitor) const {
        return visitor.visit_variable_decl(*this);
    }


    FunctionDecl::FunctionDecl(Semantics::TypeInfo typing_, const std::vector<ArgDecl>& args_, const Frontend::Token& name_, StmtPtr body_) noexcept
    : typing {std::move(typing_)}, args {args_}, name {name_}, body {std::move(body_)} {}

    bool FunctionDecl::is_directive() const noexcept {
        return false;
    }

    bool FunctionDecl::is_declarative() const noexcept {
        return true;
    }

    bool FunctionDecl::is_control_flow() const noexcept {
        return false;
    }

    bool FunctionDecl::is_expr_stmt() const noexcept {
        return false;
    }

    Semantics::TypeInfo FunctionDecl::possible_result_type() const noexcept {
        return typing;
    }

    void FunctionDecl::accept_visitor(StmtVisitor<void>& visitor) const {
        visitor.visit_function_decl(*this);
    }

    std::any FunctionDecl::accept_visitor(StmtVisitor<std::any>& visitor) const {
        return visitor.visit_function_decl(*this);
    }


    ExprStmt::ExprStmt(ExprPtr inner_) noexcept
    : inner {std::move(inner_)} {}

    bool ExprStmt::is_directive() const noexcept {
        return false;
    }

    bool ExprStmt::is_declarative() const noexcept {
        return false;
    }

    bool ExprStmt::is_control_flow() const noexcept {
        return false;
    }

    bool ExprStmt::is_expr_stmt() const noexcept {
        return true;
    }

    Semantics::TypeInfo ExprStmt::possible_result_type() const noexcept {
        return (inner->yields_value())
        ? inner->type_tagging()
        : Semantics::NullType {};
    }

    void ExprStmt::accept_visitor(StmtVisitor<void>& visitor) const {
        visitor.visit_expr_stmt(*this);
    }

    std::any ExprStmt::accept_visitor(StmtVisitor<std::any>& visitor) const {
        return visitor.visit_expr_stmt(*this);
    }


    Block::Block(std::vector<StmtPtr> stmts_) noexcept
    : stmts {std::move(stmts_)} {}

    bool Block::is_directive() const noexcept {
        return false;
    }

    bool Block::is_declarative() const noexcept {
        return false;
    }

    bool Block::is_control_flow() const noexcept {
        return true;
    }

    bool Block::is_expr_stmt() const noexcept {
        return false;
    }

    Semantics::TypeInfo Block::possible_result_type() const noexcept {
        return Semantics::NullType {};
    }

    void Block::accept_visitor(StmtVisitor<void>& visitor) const {
        visitor.visit_block(*this);
    }

    std::any Block::accept_visitor(StmtVisitor<std::any>& visitor) const {
        return visitor.visit_block(*this);
    }


    Return::Return(ExprPtr result_expr_) noexcept
    : result_expr {std::move(result_expr_)} {}

    bool Return::is_directive() const noexcept {
        return false;
    }

    bool Return::is_declarative() const noexcept {
        return false;
    }

    bool Return::is_control_flow() const noexcept {
        return true;
    }

    bool Return::is_expr_stmt() const noexcept {
        return false;
    }

    Semantics::TypeInfo Return::possible_result_type() const noexcept {
        return (result_expr->yields_value())
            ? result_expr->type_tagging()
            : Semantics::NullType {};
    }

    void Return::accept_visitor(StmtVisitor<void>& visitor) const {
        visitor.visit_return(*this);
    }

    std::any Return::accept_visitor(StmtVisitor<std::any>& visitor) const {
        return visitor.visit_return(*this);
    }


    If::If(ExprPtr test_, StmtPtr truthy_body_, StmtPtr falsy_body_) noexcept
    : test {std::move(test_)}, truthy_body {std::move(truthy_body_)}, falsy_body {std::move(falsy_body_)} {};

    bool If::is_directive() const noexcept {
        return false;
    }

    bool If::is_declarative() const noexcept {
        return false;
    }

    bool If::is_control_flow() const noexcept {
        return true;
    }

    bool If::is_expr_stmt() const noexcept {
        return false;
    }

    Semantics::TypeInfo If::possible_result_type() const noexcept {
        return Semantics::NullType {};
    }

    void If::accept_visitor(StmtVisitor<void>& visitor) const {
        visitor.visit_if(*this);
    }

    std::any If::accept_visitor(StmtVisitor<std::any>& visitor) const {
        return visitor.visit_if(*this);
    }


    While::While(ExprPtr test_, StmtPtr body_) noexcept
    : test {std::move(test_)}, body {std::move(body_)} {}

    bool While::is_directive() const noexcept {
        return false;
    }

    bool While::is_declarative() const noexcept {
        return false;
    }

    bool While::is_control_flow() const noexcept {
        return true;
    }

    bool While::is_expr_stmt() const noexcept {
        return false;
    }

    Semantics::TypeInfo While::possible_result_type() const noexcept {
        return {
            Semantics::NullType {}
        };
    }

    void While::accept_visitor(StmtVisitor<void>& visitor) const {
        visitor.visit_while(*this);
    }

    std::any While::accept_visitor(StmtVisitor<std::any>& visitor) const {
        return visitor.visit_while(*this);
    }

}