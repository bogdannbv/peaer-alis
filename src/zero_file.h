#ifndef ALIS_ZERO_FILE_H
#define ALIS_ZERO_FILE_H

#include <iostream>
#include <fstream>
#include <random>
#include <memory>

class zero_file;

typedef std::shared_ptr<zero_file> zero_file_sptr;

class zero_file {
public:
    zero_file();
    ~zero_file();

    std::string get_path();

    static std::string get_random_tmp_file(int len);
private:
    std::string path;
};


#endif //ALIS_ZERO_FILE_H
