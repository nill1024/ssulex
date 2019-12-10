
#ifndef _COMMON_HPP_
#define _COMMON_HPP_

// c
#include <cstdio>
#include <cstdlib>

namespace ssulex {
    void trim(std::string &s);
    std::string trim(std::string &&s);
    void remove_spaces(std::string &s);
    std::string remove_spaces(std::string &&s);
    std::string file2string(std::string path);
    void string2file(std::string path, std::string contents);

    template<typename... Args>
    void report_error(const char *format, Args ...args) {
        fprintf(stderr, format, args...);
        fprintf(stderr, "\n");
        exit(1);
    }
}

#endif
