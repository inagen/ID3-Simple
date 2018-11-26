#pragma once
#include <string>

namespace id3 {

std::string read_file(const std::string&);
bool write_file(const std::string&, const std::string&);

}

