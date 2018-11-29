#include "id3.h"
#include <iostream>



int main() {
	auto str = id3::read_file("renai.mp3");
	auto it = str.begin();
	id3::frame frame = id3::get_next_frame(str, it);
	std::cout << frame.frame_id << std::endl;
}