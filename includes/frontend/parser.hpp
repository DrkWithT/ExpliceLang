#pragma once

#include <string_view>
#include <vector>
#include "frontend/token.hpp"
#include "frontend/lexer.hpp"
#include "semantics/tags.hpp"
#include "syntax/exprs.hpp"
#include "syntax/stmts.hpp"

namespace XLang::Frontend {
    struct ParseError {
        std::string msg;
        Token culprit;
    };
    
    struct ParseDump {
        std::vector<Syntax::StmtPtr> decls;
        std::vector<ParseError> errors;
    };

    class Parser {
    private:
        Lexer m_lexer;
        std::vector<Token> m_window;
        std::vector<ParseError> m_errors;
        static constexpr int m_peek_count = 4;

        [[nodiscard]] constexpr Token peek_at(int forward_offset) const {
            return m_window.at(forward_offset);
        }

        [[nodiscard]] constexpr bool at_eof() const {
            return peek_at(0).tag == LexTag::eof;
        }

        void advance();

        [[nodiscard]] constexpr bool match_at(int forward_offset, std::same_as<LexTag> auto first, std::same_as<LexTag> auto... rest) const {
            const auto peeked_tag = peek_at(forward_offset).tag;

            return ((peeked_tag == first) || ... || (peeked_tag == rest));
        }

        void consume();
        void consume(LexTag tag);
        void recover();

        [[nodiscard]] Syntax::ExprPtr parse_literal();
        [[nodiscard]] Syntax::ExprPtr parse_access();
        [[nodiscard]] Syntax::ExprPtr parse_call();
        [[nodiscard]] Syntax::ExprPtr parse_unary();
        [[nodiscard]] Syntax::ExprPtr parse_factor();
        [[nodiscard]] Syntax::ExprPtr parse_term();
        [[nodiscard]] Syntax::ExprPtr parse_equality();
        [[nodiscard]] Syntax::ExprPtr parse_compare();
        [[nodiscard]] Syntax::ExprPtr parse_and();
        [[nodiscard]] Syntax::ExprPtr parse_or();
        [[nodiscard]] Syntax::ExprPtr parse_assign();

        [[nodiscard]] ParseDump parse_program();
        [[nodiscard]] Syntax::StmtPtr parse_top_stmt();
        [[nodiscard]] Syntax::StmtPtr parse_import();
        [[nodiscard]] Syntax::StmtPtr parse_function_decl();
        [[nodiscard]] std::vector<Syntax::ArgDecl> parse_arg_list();
        [[nodiscard]] std::any parse_type_specifier();
        [[nodiscard]] Syntax::StmtPtr parse_block();
        [[nodiscard]] Syntax::StmtPtr parse_nestable_stmt();
        [[nodiscard]] Syntax::StmtPtr parse_variable_decl();
        [[nodiscard]] Syntax::StmtPtr parse_expr_stmt();
        [[nodiscard]] Syntax::StmtPtr parse_return();
        [[nodiscard]] Syntax::StmtPtr parse_if();

    public:
        Parser(std::string_view source);

        [[nodiscard]] ParseDump operator()();
    };
}