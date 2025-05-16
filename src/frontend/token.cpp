#include <format>
#include "frontend/token.hpp"

namespace XLang::Frontend {
    std::string_view peek_lexeme(const Token& token, std::string_view source) noexcept {
        return source.substr(token.start, token.length);
    }

    std::string get_lexeme(const Token& token, std::string_view source) noexcept {
        return std::format("{}", peek_lexeme(token, source));
    }
}