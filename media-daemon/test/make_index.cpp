
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
//        std::cout << ssulex::index_fs(argv[1]) << std::endl;
        nlohmann::json i = ssulex::index_fs(argv[1]);
        std::cout << ssulex::update_index(i) << std::endl;
    }
    catch (std::filesystem::filesystem_error e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}

