
// c++
#include <iostream>
#include <fstream>
#include <filesystem>

// ssulex
#include <media.hpp>
#include <make_index.hpp>
#include <metadata_parser.hpp>

using namespace std::string_literals;

namespace ssulex {

nlohmann::json update_index(nlohmann::json from, nlohmann::json to) {
    for (auto &e : from.items()) {
        if (!to.contains(e.key())) {
            media m(e.value()["path"]);
            m.index();
            to[e.key()] = e.value();
            to[e.key()]["intra"] = m.get_segments();
            to[e.key()]["duration"] = m.get_duration();
        }
    }
    return to;
}

auto index_fs(std::string root) -> nlohmann::json {
    auto index = nlohmann::json{};

    for (auto &&it : std::filesystem::recursive_directory_iterator{root}) {
        auto path = it.path().string();
        if (it.is_regular_file() && is_video(path)) {
            auto file_name = get_file_name(path);
            auto id = make_id(file_name);

            if (not index.contains(id)) {
                index[id]["file_name"] = file_name;
                index[id]["path"] = path;
            }
        }
    }

    return index;
}

auto is_video(std::string path) -> bool {
    return path.find(".mp4") != std::string::npos
        || path.find(".mkv") != std::string::npos
        || path.find(".avi") != std::string::npos;
}

auto get_file_name(std::string path) -> std::string {
    auto pos = path.rfind('\\');
    if (pos == std::string::npos) {
        pos = path.rfind('/');
    }

    return path.substr(pos+1, path.length()-(pos+1));
}

// TODO: c++ std hash 사용 중 uuid 고려
auto make_id(std::string file_name) -> std::string {

    return std::to_string(std::hash<std::string>{}(file_name));
}

}
