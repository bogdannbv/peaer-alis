#ifndef ALIS_H
#define ALIS_H

#include <algorithm>
#include <cctype>
#include <locale>
#include <command.h>

namespace alis {

    static inline void ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));
    }

    static inline void rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), s.end());
    }

    static inline void trim(std::string &s) {
        rtrim(s);
        ltrim(s);
    }

    std::string find_songrec() {
        auto result = raymii::Command::exec("which songrec");
        if (result.exitstatus != 0) {
            return "";
        }

        trim(result.output);

        return result.output;
    }
}

#endif //ALIS_H
