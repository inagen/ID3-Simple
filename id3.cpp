#include "id3.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <stdint.h>


std::string id3::read_file(const std::string& filename) {

	std::ifstream fin;
	fin.open(filename);

	if(!fin.is_open()) {
		std::cout << "Error: Unable to open file " << filename << std::endl;
		return std::string();
	} else {
		std::stringstream sstream;
		sstream << fin.rdbuf();
		fin.close();
		return sstream.str();
	}
}

bool id3::write_file(const std::string& filename, const std::string& buf) {
	std::ofstream fout;
	fout.open(filename);
	auto len = filename.length();

	if(!fout.is_open()) {
		std::cout << "Error: Unable to open file " << filename << std::endl;
		return 1;
	} else {
		fout.write(filename.c_str(), len);
		fout.close();
		return 0;
	}
}

id3::header id3::get_id3_header(const std::string& buf) {
	id3::header header;
	std::string sig = buf.substr(0, 3);
	if(sig != "ID3") {
		std::cout << "Error: ID3 tag not found" << std::endl;
		return id3::header();
	} else {
		std::copy(std::begin(sig), std::end(sig), std::begin(header.sig));
		header.ver_major = buf[3];
		header.ver_revision = buf[4];
		header.flags = buf[5];
		header.size = size_to_uint32(std::bitset<32>(buf.substr(6, 10).c_str()));
		return header;
	}
}