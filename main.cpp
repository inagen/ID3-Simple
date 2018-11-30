#include "id3.h"
#include <iostream>



int main(int argc, char** argv) {
	if(argc < 3) {
		std::cout << "Please, enter arguments and filename." << std::endl;
		return 0;
	}
	std::string operation = argv[1]; 
	if(operation == "-r") {
		std::string filename = argv[2];
		std::string file = id3::read_file(filename);
		auto frames = id3::get_frames(file);
	}

}