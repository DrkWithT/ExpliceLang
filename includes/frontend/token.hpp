#pragma once

#include <string_view>
#include <string>

namespace XLang::Frontend {
    enum class LexTag {
        unknown,
        spaces,
        comment,
        identifier,
        keyword,
        type_specifier,
        // literal_null,
        literal_true,
        literal_false,
        literal_int,
        literal_float,
        literal_string,
        symbol_access,
        symbol_minus,
        symbol_plus,
        symbol_times,
        symbol_slash,
        symbol_equals,
        symbol_bang_equals,
        symbol_lesser,
        symbol_greater,
        symbol_and,
        symbol_or,
        symbol_assign,
        left_paren,
        right_paren,
        left_brack,
        right_brack,
        left_brace,
        right_brace,
        comma,
        colon,
        semicolon,
        eof
    };
    
    struct Token {
        LexTag tag;
        int start;
        int length;
        int line;
        int column;
    };

    [[nodiscard]] constexpr bool check_lex_tag_for(const Token& token, std::same_as<LexTag> auto&& first, std::same_as<LexTag> auto&& ...rest) noexcept {
        return ((token.tag == first) || ... || (token.tag == rest));
    }

    [[nodiscard]] std::string_view peek_lexeme(const Token&, std::string_view) noexcept;

    [[nodiscard]] std::string get_lexeme(const Token&, std::string_view) noexcept;
}