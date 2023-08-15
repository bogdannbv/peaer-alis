#ifndef ALIS_UTILS_H
#define ALIS_UTILS_H

#include <algorithm>
#include <cctype>
#include <locale>
#include <command.h>

namespace utils {

    inline void ltrim(std::string &s);

    inline void rtrim(std::string &s);

    inline void trim(std::string &s);

    std::string find_songrec();

    bool check_dir_or_create(const std::string &dir);
}

#endif //ALIS_UTILS_H
