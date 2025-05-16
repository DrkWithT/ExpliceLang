#include <initializer_list>
#include "frontend/token.hpp"
#include "frontend/lexer.hpp"

namespace XLang::Frontend {
    static std::initializer_list<LexicalEntry> entries = {
        LexicalEntry {"import", LexTag::keyword},
        {"func", LexTag::keyword},
        {"import", LexTag::keyword},
        {"if", LexTag::keyword},
        {"else", LexTag::keyword},
        {"return", LexTag::keyword},
        {"const", LexTag::keyword},
        {"let", LexTag::keyword},
        {"bool", LexTag::type_specifier},
        {"int", LexTag::type_specifier},
        {"float", LexTag::type_specifier},
        {"true", LexTag::literal_true},
        {"false", LexTag::literal_false},
        {"::", LexTag::symbol_access},
        {"-", LexTag::symbol_minus},
        {"+", LexTag::symbol_plus},
        {"*", LexTag::symbol_times},
        {"/", LexTag::symbol_slash},
        {"==", LexTag::symbol_equals},
        {"!=", LexTag::symbol_bang_equals},
        {"<", LexTag::symbol_lesser},
        {">", LexTag::symbol_greater},
        {"&&", LexTag::symbol_and},
        {"||", LexTag::symbol_or},
        {"=", LexTag::symbol_assign}
    };

    Lexer::Lexer(std::string_view source)
    : m_items {}, m_viewed {source}, m_pos {0}, m_end (source.size()), m_line {1}, m_col {1} {
        for (const auto& [sample, tag] : entries) {
            m_items[sample] = tag;
        }
    }

    std::string_view Lexer::view_source() const noexcept {
        return m_viewed;
    }

    Token Lexer::operator()() {
        if (at_end()) {
            return {
                .tag = LexTag::eof,
                .start = m_end,
                .length = 1,
                .line = m_line,
                .column = m_col
            };
        }

        auto symbol_peeked = m_viewed[m_pos];

        switch (symbol_peeked) {
        case '#':
            return lex_between('#', LexTag::comment);
        case '"':
            return lex_between('"', LexTag::literal_string);
        case '(':
            return lex_single(LexTag::left_paren);
        case ')':
            return lex_single(LexTag::right_paren);
        case '[':
            return lex_single(LexTag::left_brack);
        case ']':
            return lex_single(LexTag::right_brack);
        case '{':
            return lex_single(LexTag::left_brace);
        case '}':
            return lex_single(LexTag::right_brace);
        case ',':
            return lex_single(LexTag::comma);
        case ':':
            return lex_single(LexTag::colon);
        case ';':
            return lex_single(LexTag::semicolon);
        default:
            break;
        }

        if (match_spacing(symbol_peeked)) {
            return lex_spacing();
        } else if (match_digit(symbol_peeked)) {
            return lex_number();
        } else if (match_alpha(symbol_peeked)) {
            return lex_word();
        } else if (match_operator(symbol_peeked)) {
            return lex_operator();
        }

        return lex_single(LexTag::unknown);
    }

    bool Lexer::at_end() const noexcept {
        return m_pos >= m_end;
    }

    void Lexer::update_source_pos(char symbol) noexcept {
        if (symbol == '\n') {
            ++m_line;
            m_col = 1;
        } else {
            ++m_col;
        }
    }

    Token Lexer::lex_single(LexTag tag) noexcept {
        auto start = m_pos;
        const auto line = m_line;
        const auto col = m_col;

        update_source_pos(m_viewed[start]);
        ++m_pos;

        return {
            .tag = tag,
            .start = start,
            .length = 1,
            .line = line,
            .column = col
        };
    }

    Token Lexer::lex_between(char delim, LexTag tag) noexcept {
        update_source_pos(m_viewed[m_pos]);
        ++m_pos;

        auto start = m_pos;
        auto len = 0;
        const auto line = m_line;
        const auto col = m_col;

        while (!at_end()) {
            const auto symbol = m_viewed[m_pos];
            
            if (symbol == delim) {
                update_source_pos(m_viewed[m_pos]);
                break;
            }

            update_source_pos(symbol);
            ++m_pos;
            ++len;
        }

        return {
            .tag = tag,
            .start = start,
            .length = len,
            .line = line,
            .column = col
        };
    }

    Token Lexer::lex_spacing() noexcept {
        auto start = m_pos;
        auto len = 0;
        const auto line = m_line;
        const auto col = m_col;

        while (!at_end()) {
            const auto symbol = m_viewed[m_pos];
            
            if (!match_spacing(symbol)) {
                break;
            }
            
            update_source_pos(symbol);
            ++m_pos;
            ++len;
        }

        return {
            .tag = LexTag::spaces,
            .start = start,
            .length = len,
            .line = line,
            .column = col
        };
    }

    Token Lexer::lex_number() noexcept {
        auto dots = 0;
        auto start = m_pos;
        auto len = 0;
        const auto line = m_line;
        const auto col = m_col;

        while (!at_end()) {
            const auto symbol = m_viewed[m_pos];
            
            if (!match_digit(symbol) && symbol != '.') {
                break;
            }
            
            if (symbol == '.') {
                ++dots;
            }
            
            update_source_pos(symbol);
            ++m_pos;
            ++len;
        }

        LexTag deduced_tag = LexTag::unknown;

        switch (dots) {
            case 0:
                deduced_tag = LexTag::literal_int;
                break;
            case 1:
                deduced_tag = LexTag::literal_float;
                break;
            default:
                break;
        }

        return {
            .tag = deduced_tag,
            .start = start,
            .length = len,
            .line = line,
            .column = col
        };
    }

    Token Lexer::lex_word() {
        auto start = m_pos;
        auto len = 0;
        const auto line = m_line;
        const auto col = m_col;

        while (!at_end()) {
            const auto symbol = m_viewed[m_pos];
            
            if (!match_alpha(symbol)) {
                break;
            }
            
            update_source_pos(symbol);
            ++m_pos;
            ++len;
        }

        Token temp {
            .tag = LexTag::identifier,
            .start = start,
            .length = len,
            .line = line,
            .column = col
        };

        std::string_view lexeme_view = peek_lexeme(temp, m_viewed);

        if (const auto tag_it = m_items.find(lexeme_view); tag_it != m_items.cend()) {
            temp.tag = m_items.at(lexeme_view);
        }

        return temp;
    }

    Token Lexer::lex_operator() {
        auto start = m_pos;
        auto len = 0;
        const auto line = m_line;
        const auto col = m_col;

        while (!at_end()) {
            const auto symbol = m_viewed[m_pos];
            
            if (!match_operator(symbol)) {
                break;
            }
            
            update_source_pos(symbol);
            ++m_pos;
            ++len;
        }

        Token temp {
            .tag = LexTag::unknown,
            .start = start,
            .length = len,
            .line = line,
            .column = col
        };

        std::string_view lexeme_view = peek_lexeme(temp, m_viewed);

        if (const auto tag_it = m_items.find(lexeme_view); tag_it != m_items.cend()) {
            temp.tag = m_items.at(lexeme_view);
        }

        return temp;
    }
}
