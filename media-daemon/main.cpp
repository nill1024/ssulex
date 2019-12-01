
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

std::string file2string(std::string path) {
    FILE *fp = fopen(path.c_str(), "rb");
    if (fp == NULL)
        ssulex::report_error("Failed to open %s.", path);

    if (fseek(fp, 0, SEEK_END) != 0)
        ssulex::report_error("Failed to seek to the end.");

    long len = ftell(fp);
    if (len == -1L)
        ssulex::report_error("Failed to get the file size.");

    if (fseek(fp, 0, SEEK_SET) != 0)
        ssulex::report_error("Failed to seek to the start.");

    std::string s;
    s.resize(len);

    if (fread(s.data(), 1, len, fp) != len)
        ssulex::report_error("Failed to read the whole file.");

    fclose(fp);
    return s;
}

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
    auto setting = nlohmann::json::parse(file2string(argv[1]));
    start_server(setting);
    return 0;
}
