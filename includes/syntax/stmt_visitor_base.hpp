#pragma once

namespace XLang::Syntax {
    struct NativeUse;
    struct Import;
    struct VariableDecl;
    struct FunctionDecl;
    struct ExprStmt;
    struct Block;
    struct Return;
    struct If;
    struct While;

    template <typename Result>
    class StmtVisitor {
    public:
        virtual ~StmtVisitor() = default;

        virtual Result visit_native_use(const NativeUse& stmt) = 0;
        virtual Result visit_import(const Import& stmt) = 0;
        virtual Result visit_variable_decl(const VariableDecl& stmt) = 0;
        virtual Result visit_function_decl(const FunctionDecl& stmt) = 0;
        virtual Result visit_expr_stmt(const ExprStmt& stmt) = 0;
        virtual Result visit_block(const Block& stmt) = 0;
        virtual Result visit_return(const Return& stmt) = 0;
        virtual Result visit_if(const If& stmt) = 0;
        virtual Result visit_while(const While& stmt) = 0;
    };
}