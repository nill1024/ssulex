
// c
#include <cstdio>

// c++
#include <iostream>
#include <string>

// third-party
#include <nlohmann/json.hpp>
#include <yhirose/httplib.h>

// ssulex
#include <common.hpp>

void start_server(nlohmann::json &setting) {
    httplib::Server server;

    try {
        std::string path = setting["static"].get<std::string>();
        server.set_base_dir(path.c_str());
        std::cout << "Mapping " << path << " to /." << std::endl;
    }
    catch (nlohmann::detail::type_error e) {
        ssulex::report_error("static was not set in the setting.");
    }

    server.listen("0.0.0.0", 8888);
}

int main(int argc, char **argv) {
    if (argc != 2)
        ssulex::report_error("Usage : %s <setting.json>", argv[0]);
    auto setting = nlohmann::json::parse(ssulex::file2string(argv[1]));
    start_server(setting);
    return 0;
}
