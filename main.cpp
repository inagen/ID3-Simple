#include "id3.h"
#include <iostream>



int main() {
	auto str = id3::read_file("renai.mp3");
	auto header = id3::get_id3_header(str);
	auto footer = id3::get_footer_from_header(header);
	std::cout << header.sig << std::endl;
	std::cout << "Ver. 2."<< int(header.ver_major) << ".";
	std::cout << int(header.ver_revision) << std::endl;
	std::cout << header.flags << std::endl;
	std::cout << header.size << std::endl;
	std::cout << std::endl;
	std::string file;
	id3::set_footer(footer, file);
	std::cout << file << std::endl;
}