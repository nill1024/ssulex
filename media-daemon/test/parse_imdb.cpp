
// c
#include <cstdio>
#include <cstdlib>

// c++
#include <iostream>

// ssulex
#include <metadata_parser.hpp>

char *read_all(const char *path) {
    FILE *fp = fopen(path, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Failed to open %s.\n", path);
        exit(1);
    }

    if (fseek(fp, 0, SEEK_END) != 0) {
        fprintf(stderr, "Failed to seek to the end.");
        exit(1);
    }

    long len = ftell(fp);
    if (len == -1L) {
        fprintf(stderr, "Failed to get the file size.");
        exit(1);
    }

    if (fseek(fp, 0, SEEK_SET) != 0) {
        fprintf(stderr, "Failed to seek to the start.");
        exit(1);
    }

    char *buf = new char[len+1];
    if (buf == NULL) {
        fprintf(stderr, "Failed to allocate %ld.\n", len+1);
        exit(1);
    }

    if (fread(buf, 1, len, fp) != len) {
        fprintf(stderr, "Failed to read the whole file.");
        exit(1);
    }
    buf[len] = 0;

    fclose(fp);
    return buf;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <imdb_html>\n", argv[0]);
        exit(1);
    }

    char *html = read_all(argv[1]);
    imdb_parser imdb(html);
    for (std::string &q : imdb.help()) {
        try {
            auto r = imdb.query(q.c_str());
            std::cout << q << " : ";
            for (std::string &v : r) {
                std::cout << v << " ";
            }
            std::cout << std::endl;
        }
        catch (metadata_parser_error e) {
            std::cerr << e.message << std::endl;
            std::cerr << "Failed to locate " << q << "." << std::endl;
        }
    }
    free(html);
    return 0;
}

