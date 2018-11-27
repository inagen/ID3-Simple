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
		std::cout << "Error: ID3 tag not found." << std::endl;
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

id3::extended_header id3::get_id3_extended_header(const std::string& buf) {
	auto header = id3::get_id3_header(buf);
	if(!header.flags[1]) {
		std::cout << "Error: ID3 extended header not found." << std::endl;
		return id3::extended_header();
	} else {
		id3::extended_header exheader;
		exheader.size = size_to_uint32(std::bitset<32>(buf.substr(10, 14).c_str()));
		exheader.number_of_flag_bytes = buf[14]; 
		exheader.flags = buf[15];

		if(exheader.flags[1])
			exheader.is_an_update = true;
		else
			exheader.is_an_update = false;
		
		if(exheader.flags[2]) {
			exheader.is_crc_data = true;
			exheader.crc_data = size_to_uint32(std::bitset<40>(buf.substr(16, 21).c_str()));
		} else {
			exheader.is_crc_data = false;
		}
		if(exheader.flags[3]) {
			exheader.is_tag_restrictions = true;
			std::bitset<8> rests = buf[21];
			exheader.rests.tag_size = std::bitset<2>(rests.to_string().substr(0, 2)).to_ulong();
			exheader.rests.text_encoding = rests[2];
			exheader.rests.text_field_size = std::bitset<2>(rests.to_string().substr(3, 5)).to_ulong();
			exheader.rests.image_encoding = rests[5];
			exheader.rests.image_size = std::bitset<2>(rests.to_string().substr(6, 8)).to_ulong();
		} else {
			exheader.is_tag_restrictions = false;
		}
		return exheader;
	}
}

id3::footer id3::get_footer_from_header(const id3::header& header) {	
	id3::footer footer;
	footer.sig = std::copy(std::begin(header.sig), std::end(header.sig), std::begin(footer.sig));
	footer.ver_major = header.ver_major;
	footer.ver_revision = header.ver_revision;
	footer.flags = header.flags;
	footer.size = header.size;
	return footer;
}