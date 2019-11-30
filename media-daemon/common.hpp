
#ifndef _COMMON_HPP_
#define _COMMON_HPP_

namespace ssulex {
    void trim(std::string &s);
    std::string trim(std::string &&s);
    void remove_spaces(std::string &s);
    std::string remove_spaces(std::string &&s);
}

#endif
