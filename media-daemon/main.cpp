
// c
#include <cstdio>

// c++
#include <iostream>
#include <string>
#include <filesystem>

// third-party
#include <nlohmann/json.hpp>
#include <yhirose/httplib.h>

// ssulex
#include <common.hpp>
#include <make_index.hpp>

nlohmann::json setting;
nlohmann::json raw;

void load_context(std::string path) {
    setting = nlohmann::json::parse(ssulex::file2string(path));

    if (!setting.contains("media"))
        ssulex::report_error("Failed to locate the media directory.");
    if (!setting.contains("config"))
        ssulex::report_error("Failed to locate the config path.");

    std::string raw_path = setting["config"].get<std::string>() + "/raw.json";
    raw = {};
    if (std::filesystem::exists(std::filesystem::path(raw_path)))
        raw = nlohmann::json::parse(ssulex::file2string(raw_path));
    raw = ssulex::update_index(ssulex::index_fs(setting["media"]), raw);
    ssulex::string2file(raw_path, raw.dump());

    std::cout << "Finished loading contexts." << std::endl;
}

void start_server(void) {
    httplib::Server server;

    if (setting.contains("static")) {
        std::string path = setting["static"].get<std::string>();
        server.set_base_dir(path.c_str());
        std::cout << "static file: " << path << "-> /" << std::endl;
    }

    server.listen("0.0.0.0", 8888);
}

int main(int argc, char **argv) {
    if (argc != 2)
        ssulex::report_error("Usage : %s <setting.json>", argv[0]);
    load_context(argv[1]);
    start_server();
    return 0;
}
