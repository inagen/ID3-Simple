#include "id3.h"
#include <iostream>



int main() {
	auto str = id3::read_file("renai.mp3");
	auto header = id3::get_id3_header(str);
	auto footer = id3::get_footer_from_header(header);
	std::string file;
	id3::set_footer(footer, file);
	std::cout << file.length() << std::endl;
	id3::set_header(header, file);
	std::cout << file.length() << std::endl;
	std::cout << file << std::endl;
}