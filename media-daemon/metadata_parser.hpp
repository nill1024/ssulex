
#ifndef _METADATA_PARSER_HPP_
#define _METADATA_PARSER_HPP_

// c
#include <cstdio>

// c++
#include <string>
#include <vector>
#include <map>

// gumbo-query
#include <gq/Document.h>
#include <gq/Node.h>

struct metadata_parser_error
{
    std::string message;
    template<typename... Args>
    metadata_parser_error(const char *format, Args ...args) {
        int length = snprintf(NULL, 0, format, args...);
        message.resize(length);
        sprintf(message.data(), format, args...);
    }
    metadata_parser_error(const metadata_parser_error &e) : message(e.message) {}
    metadata_parser_error(metadata_parser_error &&e) : message(std::move(e.message)) {}
};

class metadata_parser {
    virtual std::vector<std::string> query(const char *key) = 0;

    // Get the list of valid queries.
    virtual std::vector<std::string> help(void) = 0;
};

class imdb_parser : public metadata_parser
{
    // We parse HTML here.
    CDocument dom;
    
    // What query corresponds to what selector?
    std::map<std::string, std::string> selectors;

    // Default behaviors are not applicable to year & thumbnail.
    std::vector<std::string> get_year(CSelection selection);
    std::vector<std::string> get_thumbnail(CSelection selection);

public:

    imdb_parser(const char *html) {
        dom.parse(html);
        selectors.emplace("synopsis", ".plot-description");
        selectors.emplace("genre", ".media-body span[itemprop=genre]");
        selectors.emplace("year", ".media-body h1 small");
        selectors.emplace("title", ".media-body h1 :not(small)");
        selectors.emplace("thumbnail", "img[alt$=Poster]");
        selectors.emplace("creator", "#cast-and-crew a[itemprop=creator] span[itemprop=name]");
        selectors.emplace("director", "#cast-and-crew a[itemprop=director] span[itemprop=name]");
        selectors.emplace("cast", "#cast-and-crew ul strong");
    }

    std::vector<std::string> query(const char *key);
    std::vector<std::string> help(void) {
        std::vector<std::string> v;
        for (auto &p : selectors)
            v.emplace_back(p.first);
        return v;
    }
};

#endif

