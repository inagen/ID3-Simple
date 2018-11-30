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
bool write_file(const std::string&, const std::string&);

header get_id3_header(const std::string&);
void set_header(const header&, std::string&);

extended_header get_id3_extended_header(const std::string&);
void set_extended_header(const extended_header&, std::string&);

footer get_footer_from_header(const header&);
void set_footer(footer&, std::string&);

frame get_next_frame(const std::string&, std::string::iterator&); 
void set_frame(const frame&, std::string&);
frame make_frame(const std::string&, const std::string&, const std::string&);											 
void print_frame(const frame&);																	
std::vector<frame> get_frames(std::string&);
bool is_frame_here(const frame&, std::string&);

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

template<unsigned N>
std::vector<uint8_t> encode(uint64_t value) {
	uint64_t result = 0;
	for (unsigned i = N-1; i < N; --i) {
		result |= (value & (0x7f << (i*7))) << i;
	}
	std::vector<uint8_t> data(N);
	for (unsigned i = N-1; i < N; --i) {
	data[i] = result & (0xff << (i*8));
	}
	return data;
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

bool write_file(const std::string& filename, const std::string& buf) {
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

void set_header(const header& header, std::string& buf) {
	std::string string_header;
	string_header += "ID3";
	string_header += static_cast<char>(header.ver_major);
	string_header += static_cast<char>(header.ver_revision);
	string_header += static_cast<char>(header.flags.to_ulong());

	auto encoded_size = encode<4>(static_cast<uint64_t>(header.size));
	std::string size(encoded_size.begin(), encoded_size.end());
	string_header += size;

	buf.replace(0, 10, string_header);
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

void set_extended_header(const extended_header& exheader, std::string& buf) {
	auto header = get_id3_header(buf);
	auto footer = get_footer_from_header(header);
	
	if(header.flags[1] == 0) {
		header.flags[1] = true;
		set_header(header, buf);
		footer.flags[1] = true;
		set_footer(footer, buf);
	}

	std::string string_exheader;
	auto encoded_size = encode<4>(static_cast<uint64_t>(exheader.size));
	std::string size(encoded_size.begin(),encoded_size.end());
	string_exheader += size;
	string_exheader += static_cast<char>(exheader.number_of_flag_bytes);
	string_exheader += static_cast<char>(exheader.flags.to_ulong());
	
	if(exheader.flags[2]) {
		auto encoded_data = encode<5>(exheader.crc_data);
		std::string string_data(encoded_data.begin(), encoded_data.end());
		string_exheader += string_data;
	} 
	if(exheader.flags[3]) {
		string_exheader += static_cast<char>(exheader.rests.to_ulong());
	}

	buf.insert(buf.begin()+10, string_exheader.begin(), string_exheader.end());

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

void set_footer(footer& footer, std::string& buf) {
	auto header = get_id3_header(buf);
	if(!footer.flags[3] || !header.flags[3]) {
		header.flags[1] = true;
		set_header(header, buf);
		footer.flags[3] = true;
	}

	buf += "3DI";
	buf += static_cast<char>(footer.ver_major);
	buf += static_cast<char>(footer.ver_revision);
	buf += static_cast<char>(footer.flags.to_ulong());
	auto encoded_size = encode<4>(static_cast<uint64_t>(footer.size));
	std::string size(encoded_size.begin(), encoded_size.end());
	buf += size;
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

void set_frame(const frame& frame, std::string& buf) {
	std::string strframe;
	auto header = get_id3_header(buf);

	strframe += frame.frame_id;
	auto encoded_size = encode<4>(static_cast<uint64_t>(frame.size));
	std::string size(encoded_size.begin(), encoded_size.end());
	
	strframe += size;
	header.size += frame.size;
	set_header(header, buf);
	
	strframe += frame.flags;
	strframe += frame.content;
	auto index = 10;
	if(header.flags[1]) {
			auto exheader = get_id3_extended_header(buf);
			index += 6;
			index += exheader.flags[2] ? 5 : 0;
			index += exheader.flags[3] ? 1 : 0;
	}
	buf.insert(index, strframe);
}

void print_frame(const frame& frame) {
	std::cout << frame.frame_id << ": ";
	std::cout << frame.content << std::endl; 
}

frame make_frame(const std::string& id, const std::string& cont, const std::string& flags) {
	uint32_t size = 4 + 2 + cont.length();
	id3::frame frame;
	frame.frame_id= id;
	frame.flags = flags;
	frame.content = cont;
	frame.size = size;
	return frame;
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

bool is_frame_here(const frame& frame, std::string& file) {
	auto frames = get_frames(file);
	auto it = std::find_if(frames.begin(), frames.end(), [=](id3::frame f) -> bool {
		return f.frame_id == frame.frame_id;
	});
	return it != frames.begin();
}

}

