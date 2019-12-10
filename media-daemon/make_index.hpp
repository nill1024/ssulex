
#ifndef _MAKE_INDEX_HPP_
#define _MAKE_INDEX_HPP_ 

// ssulex
#include <nlohmann/json.hpp>

namespace ssulex {

nlohmann::json update_index(nlohmann::json from, nlohmann::json to = {});
auto index_fs(std::string root) -> nlohmann::json;
auto is_video(std::string path) -> bool;
auto get_file_name(std::string path) -> std::string;
auto make_id(std::string file_name) -> std::string;

}

#endif
