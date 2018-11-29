#include "id3.h"
#include <iostream>



int main() {
	auto str = id3::read_file("renai.mp3");
	auto it = str.begin();
	std::string str2;
	auto header = id3::get_id3_header(str);
	id3::set_header(header, str2);
	char flag1 = char(0);
	char flag2 = char(0);
	std::string flags;
	flags+=static_cast<char>(flag1);
	flags+=static_cast<char>(flag2);
	id3::frame frame = id3::make_frame("TALB", "lol", flags);
	id3::set_frame(frame, str2);
	std::cout << str2 << std::endl;
}