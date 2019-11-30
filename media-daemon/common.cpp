
#include <string>
#include <algorithm>
#include <cctype>
#include <common.hpp>

namespace ssulex {

    // Trim a string in place.
    void trim(std::string &s) {
        s.erase(s.begin(), std::find_if_not(s.begin(), s.end(), isspace));
        s.erase(std::find_if_not(s.rbegin(), s.rend(), isspace).base(), s.end());
    }
    
    // Return a trimmed string.
    std::string trim(std::string &&s) {
        trim(s);
        return s;
    }

    // Remove white spaces from a string.
    void remove_spaces(std::string &s) {
        s.erase(std::remove_if(s.begin(), s.end(), isspace), s.end());
    }

    // Return a string with no white spaces.
    std::string remove_spaces(std::string &&s) {
        remove_spaces(s);
        return s;
    }
};


