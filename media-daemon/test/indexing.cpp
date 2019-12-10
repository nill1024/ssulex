
extern "C" {
    #include <libavformat/avformat.h>
}

#include <indexing.hpp>
#include <iostream>

int main(int argc, char **argv)
{
    if (argc != 2) {
        std::cerr << "No file specified." << std::endl;
        return 1;
    }
    try {
        media m(argv[1]);
        m.index();
        for (int64_t timestamp : m.get_segments())
            std::cout << timestamp << std::endl;
    }
    catch (media_error e) {
        std::cerr << e.message << std::endl;
    }
    return 0;
}

