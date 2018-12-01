#include "id3.h"
#include <iostream>



int main(int argc, char** argv) {
	if(argc < 2) {
		std::cout << "Please, enter filename." << std::endl;
		return 0;
	}

	std::string filename = argv[2];
	std::string file = id3::read_file(filename);
	auto frames = id3::get_frames(file);
	for(int i = 0; i < frames.size(); i++) {
		id3::print_frame(frames[i]);
	}
}