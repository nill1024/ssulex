
// c
#include <cstring>
#include <cctype>

// c++
#include <regex>
#include <utility>
#include <algorithm>

// ssulex
#include <common.hpp>
#include <metadata_parser.hpp>

std::vector<std::string> imdb_parser::query(const char *key) {

    // Is it a valid query?
    auto i = selectors.find(key);
    if (i == selectors.end())
        throw metadata_parser_error("%s is an invalid key.", key);

    // Can we actually locate it?
    CSelection selection = dom.find(i->second);
    if (selection.nodeNum() == 0)
        throw metadata_parser_error("%s is missing.", key);
    
    // Trim & return it.
    if (strcmp(key, "year") == 0)
        return get_year(selection);
    else if (strcmp(key, "thumbnail") == 0)
        return get_thumbnail(selection);
    else {
        std::vector<std::string> v;
        for (int i=0; i<selection.nodeNum(); i++)
            v.emplace_back(ssulex::trim(selection.nodeAt(i).text()));
        return v;
    }
}

std::vector<std::string> imdb_parser::get_year(CSelection selection) {
    std::string str = ssulex::remove_spaces(selection.nodeAt(0).text());
    std::regex e("^[^(]*\\((\\d+).*?$");
    std::cmatch m;
    if (!std::regex_match(str.c_str(), m, e))
        throw metadata_parser_error("%s is an invalid input for an year.", str.c_str());
    if (!m[1].matched)
        throw metadata_parser_error("Failed to locate an year in %s.", str.c_str());
    return std::vector<std::string>(1, m[1]);
}

std::vector<std::string> imdb_parser::get_thumbnail(CSelection selection) {
    std::string thumbnail = selection.nodeAt(0).attribute("src");
    return std::vector<std::string>(1, std::move(thumbnail));
}

