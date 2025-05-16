#include <fstream>
#include <sstream>
#include "frontend/files.hpp"

namespace XLang::Frontend {
    std::string read_file(const char* file_name) {
        std::ifstream reader {file_name};
        std::ostringstream sout {};

        std::string line;

        while (std::getline(reader, line)) {
            sout << line << '\n';
            line.clear();
        }

        return sout.str();
    }
}