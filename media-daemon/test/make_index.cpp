
// c++
#include <iostream>

// ssulex
#include <common.hpp>
#include <make_index.hpp>

int main(int argc, char **argv) {
    if (argc != 2)
        ssulex::report_error("Usage: %s <media path>", argv[0]);
    
    std::cout << ssulex::make_index(argv[1]) << std::endl;
    return 0;
}

