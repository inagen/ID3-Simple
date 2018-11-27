#include "id3.h"
#include <iostream>



int main() {
	auto str = id3::read_file("renai.mp3");
	auto header = id3::get_id3_header(str);
	std::cout << header.size << std::endl;
}