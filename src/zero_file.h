//
// Created by bogdan on 8/13/23.
//

#ifndef RECORDER_ZERO_FILE_H
#define RECORDER_ZERO_FILE_H

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

private:
    std::string path;
    static std::string get_random_tmp_file(int len);
};


#endif //RECORDER_ZERO_FILE_H
