#include "zero_file.h"

zero_file::zero_file() {
    path = get_random_tmp_file(16);

    std::fstream stream(path, std::ios::out | std::ios::binary);

    for (size_t i = 0; i < 1024 * 8; i++) stream << int8_t(0);

    stream.close();

    std::cout << "Created random temp file: " << path << std::endl;
}

zero_file::~zero_file() {
    if (path.empty()) {
        std::cout<<"Path is empty"<<std::endl;
        return;
    }
    std::remove(path.c_str());
    std::cout << "Removed random temp file: " << path << std::endl;
}

std::string zero_file::get_path() {
    return path;
}

 std::string zero_file::get_random_tmp_file(const int len) {
    static const char alphanum[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<> dist(0, sizeof(alphanum) - 2);

    std::string tmp_s;
    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i) {
        std::cout<<dist(mt) % (sizeof(alphanum) - 1)<<std::endl;
        tmp_s += alphanum[dist(mt)];
    }

    return "/tmp/" + tmp_s;
}
