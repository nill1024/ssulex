
#ifndef SSULEX_MAKE_INDEX
#define SSULEX_MAKE_INDEX

#include "json.hpp"

namespace ssulex {

auto make_index(void) -> void;
auto is_video(std::string path) -> bool;
auto get_file_name(std::string path) -> std::string;
auto make_id(std::string file_name) -> std::string;
auto update_index(nlohmann::json &&index) -> void;
auto update_index(nlohmann::json &index) -> void;

}

#endif
