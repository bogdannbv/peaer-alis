#include <utils.h>
#include <sys/stat.h>
#include <iostream>

namespace utils {
    inline void ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));
    }

    inline void rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), s.end());
    }

    inline void trim(std::string &s) {
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

    bool check_dir_or_create(const std::string &dir) {
        struct stat info{};
        if (
                stat(dir.c_str(), &info) != 0
                && mkdir(dir.c_str(), 0755) == 0
        ) {
                return true;
        } else if (info.st_mode & S_IFDIR) {
            return true;
        }

        return false;
    }
}
