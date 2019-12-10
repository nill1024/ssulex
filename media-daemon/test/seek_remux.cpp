
extern "C" {
    #include <libavformat/avformat.h>
}

#include <indexing.hpp>
#include <iostream>
#include <fstream>
#include <vector>

std::vector<int64_t> read_segments(const char *path)
{
    int64_t timestamp;
    std::vector<int64_t> segments;
    std::ifstream infile(path);
    while (infile >> timestamp)
        segments.push_back(timestamp);
    infile.close();
    return segments;
}

int main(int argc, char **argv)
{
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input> <segments>" << std::endl;
        return 1;
    }
    try {
        media m(argv[1]);
        m.load_segments(read_segments(argv[2]));
//        m.remux(5);
//        m.remux(6);
        m.transcode(5);
        m.transcode(6, 10000000);
    } catch (media_error e) {
        std::cerr << e.message << std::endl;
    }
}

