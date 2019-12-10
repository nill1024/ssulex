
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

    std::string file2string(std::string path) {
        FILE *fp = fopen(path.c_str(), "rb");
        if (fp == NULL)
            ssulex::report_error("Failed to open %s.", path);
    
        if (fseek(fp, 0, SEEK_END) != 0)
            ssulex::report_error("Failed to seek to the end.");
    
        long len = ftell(fp);
        if (len == -1L)
            ssulex::report_error("Failed to get the file size.");
    
        if (fseek(fp, 0, SEEK_SET) != 0)
            ssulex::report_error("Failed to seek to the start.");
    
        std::string s;
        s.resize(len);
    
        if (fread(s.data(), 1, len, fp) != len)
            ssulex::report_error("Failed to read the whole file.");
    
        fclose(fp);
        return s;
    }

    void string2file(std::string path, std::string contents) {
        FILE *fp = fopen(path.c_str(), "wb");
        if (fp == NULL)
            ssulex::report_error("Failed to open %s.", path);

        if (fwrite(contents.c_str(), 1, contents.size(), fp) != contents.size())
            ssulex::report_error("Failed to write the string.");

        fclose(fp);
    }
};


