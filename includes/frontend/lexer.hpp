#pragma once

#include <string_view>
#include <unordered_map>
#include "frontend/token.hpp"

namespace XLang::Frontend {
    struct LexicalEntry {
        std::string_view lex_sample;
        LexTag tag;
    };

    class Lexer {    
    public:
        Lexer(std::string_view source);

        [[nodiscard]] std::string_view view_source() const noexcept;

        [[nodiscard]] Token operator()();

    private:
        static constexpr bool match_spacing(char c) noexcept {
            return c == ' ' || c == '\t' || c == '\r' || c == '\n';
        }

        /// @note The C++ standard does not guarantee the character comparison trick for [A-Z] | [a-z], but most modern systems should allow this.
        static constexpr bool match_alpha(char c) noexcept {
            return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_';
        }

        static constexpr bool match_digit(char c) noexcept {
            return c >= '0' && c <= '9';
        }

        static constexpr bool match_operator(char c) noexcept {
            return c == ':' || c == '+' || c == '-' || c == '*' || c == '/' || c == '!' || c == '=' || c == '<' || c == '>' || c == '&' || c == '|';
        }

        bool at_end() const noexcept;
        void update_source_pos(char symbol) noexcept;
        Token lex_single(LexTag tag) noexcept;
        Token lex_between(char delim, LexTag tag) noexcept;
        void skip_spaces() noexcept;
        Token lex_number() noexcept;
        Token lex_word();
        Token lex_operator();

        std::unordered_map<std::string_view, LexTag> m_items;
        std::string_view m_viewed;
        int m_pos;
        int m_end;
        int m_line;
        int m_col;
    };
}
