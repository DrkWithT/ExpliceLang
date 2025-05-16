#pragma once

#include <string>

namespace XLang::Frontend {
    [[nodiscard]] std::string read_file(const char* file_name);
}