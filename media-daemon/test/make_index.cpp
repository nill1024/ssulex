
// c++
#include <iostream>
#include <filesystem>

// ssulex
#include <common.hpp>
#include <make_index.hpp>

int main(int argc, char **argv) {
    if (argc != 2)
        ssulex::report_error("Usage: %s <media path>", argv[0]);
    
    try {
        std::cout << ssulex::make_index(argv[1]) << std::endl;
    }
    catch (std::filesystem::filesystem_error e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}

