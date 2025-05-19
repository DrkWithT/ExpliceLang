#include <utility>
#include <stdexcept>
#include "frontend/parser.hpp"
#include "frontend/token.hpp"

namespace XLang::Frontend {
    void Parser::advance() {
        m_window.erase(m_window.cbegin());

        Token temp;

        while (temp.tag != LexTag::eof) {
            temp = m_lexer();

            if (temp.tag == LexTag::spaces || temp.tag == LexTag::comment) {
                continue;
            }

            break;
        }

        m_window.emplace_back(temp);
    }

    void Parser::consume() {
        advance();
    }

    void Parser::consume(LexTag tag) {
        if (match_at(0, tag)) {
            advance();
        }

        m_errors.emplace_back("Unexpected token.", peek_at(0));
        throw std::runtime_error {"SYNTAX ERROR\n"};
    }

    void Parser::recover() {
        while (!at_eof()) {
            if (match_at(0, LexTag::semicolon)) {
                break;
            }

            consume();
        }
    }

    Syntax::ExprPtr Parser::parse_literal() {
        Token literal_slice = peek_at(0);

        if (match_at(0, LexTag::literal_true) || match_at(0, LexTag::literal_false)) {
            consume();
            return std::make_unique<Syntax::Literal>(literal_slice, Semantics::TypeTag::x_type_bool, false);
        } else if (match_at(0, LexTag::literal_int)) {
            consume();
            return std::make_unique<Syntax::Literal>(literal_slice, Semantics::TypeTag::x_type_int, false);
        } else if (match_at(0, LexTag::literal_float)) {
            consume();
            return std::make_unique<Syntax::Literal>(literal_slice, Semantics::TypeTag::x_type_float, false);
        } else if (match_at(0, LexTag::literal_string)) {
            consume();
            return std::make_unique<Syntax::Literal>(literal_slice, Semantics::TypeTag::x_type_string, false);
        } else if (match_at(0, LexTag::identifier)) {
            consume();
            return std::make_unique<Syntax::Literal>(literal_slice, Semantics::TypeTag::x_type_unknown, false);
        }

        m_errors.emplace_back("Invalid literal token!", peek_at(0));
        throw std::runtime_error {"SYNTAX ERROR\n"};
    }

    Syntax::ExprPtr Parser::parse_access() {
        auto lhs = parse_literal();

        while (!at_eof()) {
            if (!match_at(0, LexTag::symbol_access)) {
                break;
            }

            consume();

            auto rhs = parse_literal();

            lhs = std::make_unique<Syntax::Binary>(std::move(lhs), std::move(rhs), Semantics::OpTag::access);
        }

        return lhs;
    }

    Syntax::ExprPtr Parser::parse_call() {
        Token callee_id = peek_at(0);
        consume(LexTag::identifier);

        auto call_source = std::make_unique<Syntax::Literal>(callee_id, Semantics::TypeTag::x_type_unknown, true);
        std::vector<Syntax::ExprPtr> call_args;

        consume(LexTag::left_paren);

        while (!at_eof()) {
            if (match_at(0, LexTag::right_paren)) {
                consume();
                break;
            }

            call_args.emplace_back(parse_or());
            consume(LexTag::comma);
        }

        return std::make_unique<Syntax::Call>(std::move(call_args), std::move(call_source));
    }

    Syntax::ExprPtr Parser::parse_unary() {
        if (match_at(0, LexTag::left_paren)) {
            consume(LexTag::left_paren);
            auto wrapped_result = parse_or();
            consume(LexTag::right_paren);
            return wrapped_result;
        } else if (match_at(0, LexTag::symbol_minus)) {
            consume();
            auto inner = parse_access();
            return std::make_unique<Syntax::Unary>(std::move(inner), Semantics::OpTag::negate);
        } else if (match_at(0, LexTag::identifier) && match_at(1, LexTag::left_paren)) {
            return parse_call();
        } else {
            return parse_access();
        }
    }

    Syntax::ExprPtr Parser::parse_factor() {
        auto lhs = parse_unary();

        while (!at_eof()) {
            if (!match_at(0, LexTag::symbol_times, LexTag::symbol_slash)) {
                break;
            }

            const auto op = (match_at(0, LexTag::symbol_times))
                ? Semantics::OpTag::multiply
                : Semantics::OpTag::divide;
            consume();

            lhs = std::make_unique<Syntax::Binary>(std::move(lhs), parse_unary(), op);
        }

        return lhs;
    }

    Syntax::ExprPtr Parser::parse_term() {
        auto lhs = parse_factor();

        while (!at_eof()) {
            if (!match_at(0, LexTag::symbol_plus, LexTag::symbol_minus)) {
                break;
            }

            const auto op = (match_at(0, LexTag::symbol_plus))
                ? Semantics::OpTag::add
                : Semantics::OpTag::subtract;
            consume();

            lhs = std::make_unique<Syntax::Binary>(std::move(lhs), parse_factor(), op);
        }

        return lhs;
    }

    Syntax::ExprPtr Parser::parse_equality() {
        auto lhs = parse_term();

        while (!at_eof()) {
            if (!match_at(0, LexTag::symbol_equals, LexTag::symbol_bang_equals)) {
                break;
            }

            const auto op = (match_at(0, LexTag::symbol_equals))
                ? Semantics::OpTag::cmp_equ
                : Semantics::OpTag::cmp_neq;
            consume();

            lhs = std::make_unique<Syntax::Binary>(std::move(lhs), parse_term(), op);
        }

        return lhs;
    }

    Syntax::ExprPtr Parser::parse_compare() {
        auto lhs = parse_equality();

        while (!at_eof()) {
            if (!match_at(0, LexTag::symbol_lesser, LexTag::symbol_greater)) {
                break;
            }

            const auto op = (match_at(0, LexTag::symbol_lesser))
                ? Semantics::OpTag::cmp_lt
                : Semantics::OpTag::cmp_gt;
            consume();

            lhs = std::make_unique<Syntax::Binary>(std::move(lhs), parse_equality(), op);
        }

        return lhs;
    }

    Syntax::ExprPtr Parser::parse_and() {
        auto lhs = parse_compare();

        while (!at_eof()) {
            if (!match_at(0, LexTag::symbol_and)) {
                break;
            }

            consume();

            lhs = std::make_unique<Syntax::Binary>(std::move(lhs), parse_compare(), Semantics::OpTag::logic_and);
        }

        return lhs;
    }

    Syntax::ExprPtr Parser::parse_or() {
        auto lhs = parse_and();

        while (!at_eof()) {
            if (!match_at(0, LexTag::symbol_or)) {
                break;
            }

            consume();

            lhs = std::make_unique<Syntax::Binary>(std::move(lhs), parse_and(), Semantics::OpTag::logic_or);
        }

        return lhs;
    }

    Syntax::ExprPtr Parser::parse_assign() {
        auto assign_root = parse_or();

        if (match_at(0, LexTag::symbol_assign)) {
            consume();
            /// @note Assignment expr. evaluates rhs initializer 1st. By Binary node visitation which goes left then right, this should work.
            assign_root = std::make_unique<Syntax::Binary>(parse_or(), std::move(assign_root), Semantics::OpTag::assign);
        }

        return assign_root;
    }

    ParseDump Parser::parse_program() {
        std::vector<Syntax::StmtPtr> top_stmts;

        /// @note Begin parsing through top imports... This is required by the grammar rules to simplify the eventual handling of imports before any function resolving and evaluation at runtime.
        while (!at_eof()) {
            try {
                top_stmts.emplace_back(parse_import());
            } catch ([[maybe_unused]] const std::runtime_error& parse_oops) {
                recover();
            }
        }

        /// @note Also as mandated by the grammar, parse any function declarations coming after imports.
        while (!at_eof()) {
            try {
                top_stmts.emplace_back(parse_function_decl());
            } catch ([[maybe_unused]] const std::runtime_error& parse_oops_2) {
                recover();
            }
        }

        return {
            std::move(top_stmts),
            m_errors,
        };
    }

    Syntax::StmtPtr Parser::parse_import() {
        Token temp = peek_at(0);
        consume();

        if (peek_lexeme(temp, m_lexer.view_source()) != "import") {
            m_errors.emplace_back("Invalid keyword for expected import.", peek_at(0));
            throw std::runtime_error {"SYNTAX ERROR\n"};
        }

        Token unit_name = peek_at(0);
        consume(LexTag::identifier);
        consume(LexTag::semicolon);

        return std::make_unique<Syntax::Import>(unit_name);
    }

    Syntax::StmtPtr Parser::parse_function_decl() {
        Token temp = peek_at(0);
        consume();

        if (peek_lexeme(temp, m_lexer.view_source()) != "func") {
            m_errors.emplace_back("Invalid keyword for expected function decl.", peek_at(0));
            throw std::runtime_error {"SYNTAX ERROR\n"};
        }

        Token func_name = peek_at(0);
        consume(LexTag::identifier);

        std::vector<Syntax::ArgDecl> argument_decls = parse_arg_list();
        consume(LexTag::colon);

        std::any func_return_type = parse_type_specifier();

        auto func_body = parse_block();

        return std::make_unique<Syntax::FunctionDecl>(func_return_type, argument_decls, func_name, std::move(func_body));
    }

    std::vector<Syntax::ArgDecl> Parser::parse_arg_list() {
        consume(LexTag::left_paren);

        std::vector<Syntax::ArgDecl> argument_decls;

        while (!at_eof()) {
            if (match_at(0, LexTag::right_paren)) {
                consume();
                break;
            }

            Token arg_name = peek_at(0);
            consume(LexTag::identifier);
            consume(LexTag::colon);

            auto arg_type = parse_type_specifier();

            argument_decls.emplace_back(arg_type, arg_name);

            consume(LexTag::comma);
        }

        return argument_decls;
    }

    std::any Parser::parse_type_specifier() {
        if (!match_at(0, LexTag::type_specifier)) {
            m_errors.emplace_back("Invalid token for type specifier.", peek_at(0));
            throw std::runtime_error {"SYNTAX ERROR\n"};
        }

        auto peeked_type_specifier = peek_lexeme(peek_at(0), m_lexer.view_source());

        if (peeked_type_specifier == "bool") {
            consume();
            return Semantics::TypeTag::x_type_bool;
        } else if (peeked_type_specifier == "int") {
            consume();
            return Semantics::TypeTag::x_type_int;
        } else if (peeked_type_specifier == "float") {
            consume();
            return Semantics::TypeTag::x_type_float;
        } else if (peeked_type_specifier == "string") {
            consume();
            return Semantics::TypeTag::x_type_string;
        }

        m_errors.emplace_back("Unrecognized type name- may not be implemented in the compiler.", peek_at(0));
        throw std::runtime_error {"SYNTAX ERROR\n"};
    }

    Syntax::StmtPtr Parser::parse_block() {
        consume(LexTag::left_brace);

        std::vector<Syntax::StmtPtr> block_stmts;

        while (!at_eof()) {
            if (match_at(0, LexTag::right_brace)) {
                consume();
                break;
            }

            block_stmts.emplace_back(parse_nestable_stmt());
        }

        return std::make_unique<Syntax::Block>(std::move(block_stmts));
    }

    Syntax::StmtPtr Parser::parse_nestable_stmt() {
        auto starting_keyword = peek_lexeme(peek_at(0), m_lexer.view_source());

        if (starting_keyword == "let" || starting_keyword == "const") {
            return parse_variable_decl();
        } else if (starting_keyword == "if") {
            return parse_if();
        } else if (starting_keyword == "return") {
            return parse_return();
        }

        return parse_expr_stmt();
    }

    Syntax::StmtPtr Parser::parse_variable_decl() {
        auto var_pre_mark = peek_lexeme(peek_at(0), m_lexer.view_source());
        consume();

        auto var_is_readonly = var_pre_mark == "const";
        consume();

        Token var_name = peek_at(0);
        consume(LexTag::identifier);
        consume(LexTag::colon);

        std::any var_typing = parse_type_specifier();
        consume(LexTag::symbol_assign);

        auto var_initializer = parse_or();
        consume(LexTag::semicolon);

        return std::make_unique<Syntax::VariableDecl>(var_typing, var_name, std::move(var_initializer), var_is_readonly);
    }

    Syntax::StmtPtr Parser::parse_expr_stmt() {
        auto inner_expr = parse_or();
        consume(LexTag::semicolon);

        return std::make_unique<Syntax::ExprStmt>(std::move(inner_expr));
    }

    Syntax::StmtPtr Parser::parse_return() {
        consume(LexTag::keyword);

        auto result_expr = parse_or();
        consume(LexTag::semicolon);

        return std::make_unique<Syntax::Return>(std::move(result_expr));
    }

    Syntax::StmtPtr Parser::parse_if() {
        consume();

        consume(LexTag::left_paren);
        auto testing_expr = parse_or();
        consume(LexTag::right_paren);

        auto truthy_block = parse_block();
        Syntax::StmtPtr falsy_block {nullptr};

        if (peek_lexeme(peek_at(0), m_lexer.view_source()) == "else") {
            consume();
            falsy_block = parse_block();
        }

        return std::make_unique<Syntax::If>(std::move(testing_expr), std::move(truthy_block), std::move(falsy_block));
    }


    Parser::Parser(std::string_view source)
    : m_lexer {source}, m_window {}, m_errors {} {
        for (auto token_preload_count = 0; token_preload_count < m_peek_count; ++token_preload_count) {
            m_window.emplace_back(m_lexer());
        }
    }

    ParseDump Parser::operator()() {
        return parse_program();
    }
}