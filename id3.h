#pragma once
#include <string>
#include <cstring>
#include <vector>
#include <bitset>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

namespace id3 {

struct header {
    std::string sig;  		// "ID3" file identifier
	uint8_t ver_major;		// First byte of ID3v2 version
	uint8_t ver_revision;	// Second byte of ID3v2 version
	std::bitset<8> flags; 	// Flags in %abcd0000 format
	uint32_t size; 			// Size
};

struct footer {
    std::string sig; 			// "3DI" file identifier
	uint8_t ver_major;		// First byte of ID3v2 version
	uint8_t ver_revision;	// Second byte of ID3v2 version
	std::bitset<8> flags; 	// Flags in %abcd0000 format
	uint32_t size; 			// Size
};

struct extended_header {
	uint32_t size;
	uint8_t number_of_flag_bytes;		
	std::bitset<8> flags;
	uint64_t crc_data;
	std::bitset<8> rests;	
};

struct frame {
	std::string frame_id;
	uint32_t size;
	std::string flags;
	std::string content;
};

std::string read_file(const std::string&);

header get_id3_header(const std::string&);

extended_header get_id3_extended_header(const std::string&);

footer get_footer_from_header(const header&);

frame get_next_frame(const std::string&, std::string::iterator&); 										 
void print_frame(const frame&);																	
std::vector<frame> get_frames(std::string&);

template<unsigned N>
uint64_t decode(const std::string& data) {
	std::vector<bool> bites(N*8);

	auto n = 0;
	for(int i = 0; i < N; ++i) {
		std::bitset<8> byte(data[i]);
		for (int j = 0; j < 8; ++j) {
			bites[j+n] = byte[7-j];
		}
		n += 8;
	}

	for(auto it = bites.begin(); it != bites.end(); it +=8) {
		bites.erase(it);
		it--;
	}

	bites = std::vector<bool>(bites.rbegin(), bites.rend());
	std::bitset<N*7> result;
	for(int i = 0; i < bites.size(); i++) {
		result[i] = bites[i];
	}
	return static_cast<uint64_t>(result.to_ulong());
}

std::string read_file(const std::string& filename) {

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


header get_id3_header(const std::string& buf) {
	id3::header header;
	std::string sig = buf.substr(0, 3);
	if(sig != "ID3") {
		std::cout << "Error: ID3 tag not found." << std::endl;
		return id3::header();
	} else {
		header.sig = sig;
		header.ver_major = buf[3];
		header.ver_revision = buf[4];
		header.flags = buf[5];
		header.size = decode<4>(buf.substr(6,4));
		return header;
	}
}

extended_header get_id3_extended_header(const std::string& buf) {
	auto header = get_id3_header(buf);
	if(!header.flags[1]) {
		std::cout << "Error: ID3 extended header not found." << std::endl;
		return extended_header();
	} else {
		extended_header exheader;
		exheader.size = decode<4>(buf.substr(10, 4));
		exheader.number_of_flag_bytes = buf[14]; 
		exheader.flags = buf[15];
		
		if(exheader.flags[2]) //is CRC data
			exheader.crc_data = decode<5>(buf.substr(16,5));

		if(exheader.flags[3]) { // is Tag restrictions
			auto i = exheader.flags[2] ? 5 : 0;
			std::bitset<8> rests = buf[16+i];
		}

		return exheader;
	}
}

footer get_footer_from_header(const header& header) {	
	footer footer;
	std::string sig = "3DI";
	footer.sig = sig;
	footer.ver_major = header.ver_major;
	footer.ver_revision = header.ver_revision;
	footer.flags = header.flags;
	footer.size = header.size;
	return footer;
}

frame get_next_frame(const std::string& buf, std::string::iterator& it) {
	static auto tagsize = 0;
	if (it == buf.begin()) {
		auto header = get_id3_header(buf);
		tagsize = header.size;
		it += 10;
		if(header.flags[1]) {
			auto exheader = get_id3_extended_header(buf);
			it += 6;
			it += exheader.flags[2] ? 5 : 0;
			it += exheader.flags[3] ? 1 : 0;
		}
	}
	id3::frame frame;
	char kek = static_cast<char>(0);
	std::string nullstring(32, kek); 
	if((it - buf.begin() >= tagsize) || std::string(it, it+32) == nullstring) {
		frame.frame_id = "0000";
		return frame;
	}

	auto frame_id = std::string(it, it+4);
	frame.frame_id = frame_id;
	it += 4;

	auto size = decode<4>(std::string(it, it+4)) + 10;
	frame.size = size;
	size -= 8;
	it += 4;


	frame.flags = std::string(it, it+2);
	size -= 2;
	it += 2;
	std::string content;
	while(size > 0) {
		content += *it;
		it++;
		size -= 1;
	}
	frame.content = content;
	return frame;
}

void print_frame(const frame& frame) {
	std::cout << frame.frame_id << ": ";
	std::cout << frame.content << std::endl; 
}

std::vector<frame> get_frames(std::string& file) {
	std::vector<frame> frames;
	auto header = get_id3_header(file);
	auto it = file.begin();
	while(it != file.begin() + header.size) {
		auto frame = id3::get_next_frame(file, it);
		if(frame.frame_id == "0000")
			break;
		frames.push_back(frame);
	}
	return frames;
}

}

