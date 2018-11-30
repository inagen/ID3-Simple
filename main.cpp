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
		auto header = id3::get_id3_header(file);
		auto it = file.begin();
		while(it != file.begin() + header.size) {
			auto frame = id3::get_next_frame(file, it);
			if(frame.frame_id == "0000")
				break;
			id3::print_frame(frame);
		}
	}

}