#pragma once

namespace XLang::Syntax {
    struct Literal;
    struct Unary;
    struct Binary;
    struct Call;

    template <typename Result>
    class ExprVisitor {
    public:
        virtual ~ExprVisitor() = default;

        virtual Result visit_literal(const Literal& expr) = 0;
        virtual Result visit_unary(const Unary& expr) = 0;
        virtual Result visit_binary(const Binary& expr) = 0;
        virtual Result visit_call(const Call& expr) = 0;
    };
}