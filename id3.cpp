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

uint64_t id3::decode(const std::string& data) {

	auto N = data.length();
    uint64_t value = 0;
    for (unsigned i = N-1; i < N; --i) {
        value |= data[i] << (i*8);
    }
    uint64_t result = 0;
    for (unsigned i = N-1; i < N; --i) {
        result |= (value & (0xff << (i*8))) >> i;
    }
    return result;
}


id3::header id3::get_id3_header(const std::string& buf) {
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
		header.size = id3::decode(buf.substr(6,4));
		return header;
	}
}

void id3::set_header(const id3::header& header, std::string& buf) {
	std::string string_header;
	string_header += "ID3";
	string_header += static_cast<char>(header.ver_major);
	string_header += static_cast<char>(header.ver_revision);
	string_header += static_cast<char>(header.flags.to_ulong());

	auto encoded_size = id3::encode<4>(static_cast<uint64_t>(header.size));
	std::string size(encoded_size.begin(), encoded_size.end());
	string_header += size;

	buf.insert(0, string_header);
}

id3::extended_header id3::get_id3_extended_header(const std::string& buf) {
	auto header = id3::get_id3_header(buf);
	if(!header.flags[1]) {
		std::cout << "Error: ID3 extended header not found." << std::endl;
		return id3::extended_header();
	} else {
		id3::extended_header exheader;
		exheader.size = id3::decode(buf.substr(10, 4));
		exheader.number_of_flag_bytes = buf[14]; 
		exheader.flags = buf[15];
		
		if(exheader.flags[2]) //is CRC data
			exheader.crc_data = id3::decode(buf.substr(16,5));

		if(exheader.flags[3]) { // is Tag restrictions
			auto i = exheader.flags[2] ? 5 : 0;
			std::bitset<8> rests = buf[16+i];
		}

		return exheader;
	}
}

void id3::set_extended_header(const id3::extended_header& exheader, std::string& buf) {
	auto header = id3::get_id3_header(buf);
	auto footer = id3::get_footer_from_header(header);
	
	if(header.flags[1] == 0) {
		header.flags[1] = true;
		id3::set_header(header, buf);
		footer.flags[1] = true;
		id3::set_footer(footer, buf);
	}

	std::string string_exheader;
	auto encoded_size = id3::encode<4>(static_cast<uint64_t>(exheader.size));
	std::string size(encoded_size.begin(),encoded_size.end());
	string_exheader += size;
	string_exheader += static_cast<char>(exheader.number_of_flag_bytes);
	string_exheader += static_cast<char>(exheader.flags.to_ulong());
	
	if(exheader.flags[2]) {
		auto encoded_data = id3::encode<5>(exheader.crc_data);
		std::string string_data(encoded_data.begin(), encoded_data.end());
		string_exheader += string_data;
	} 
	if(exheader.flags[3]) {
		string_exheader += static_cast<char>(exheader.rests.to_ulong());
	}

	buf.insert(buf.begin()+10, string_exheader.begin(), string_exheader.end());

}

id3::footer id3::get_footer_from_header(const id3::header& header) {	
	id3::footer footer;
	std::string sig = "3DI";
	footer.sig = sig;
	footer.ver_major = header.ver_major;
	footer.ver_revision = header.ver_revision;
	footer.flags = header.flags;
	footer.size = header.size;
	return footer;
}

void id3::set_footer(id3::footer& footer, std::string& buf) {
	auto header = id3::get_id3_header(buf);
	if(!footer.flags[3] || !header.flags[3]) {
		header.flags[1] = true;
		id3::set_header(header, buf);
		footer.flags[3] = true;
	}

	buf += "3DI";
	buf += static_cast<char>(footer.ver_major);
	buf += static_cast<char>(footer.ver_revision);
	buf += static_cast<char>(footer.flags.to_ulong());
	auto encoded_size = id3::encode<4>(static_cast<uint64_t>(footer.size));
	std::string size(encoded_size.begin(), encoded_size.end());
	buf += size;
}

id3::frame id3::get_next_frame(const std::string& buf, std::string::iterator& it) {
	if (it == buf.begin()) {
		auto header = get_id3_header(buf);
		it += 10;
		if(header.flags[1]) {
			auto exheader = get_id3_extended_header(buf);
			it += 6;
			it += exheader.flags[2] ? 5 : 0;
			it += exheader.flags[3] ? 1 : 0;
		}
	}

	id3::frame frame;

	auto frame_id = std::string(it, it+4);
	frame.frame_id = frame_id;
	it += 4;

	auto size = id3::decode(std::string(it, it+4));
	frame.size = size;
	size -= 8;
	it += 4;


	frame.flags = std::string(it, it+2);
	size -= 2;
	it += 2;
	std::string content;
	while(size != 0) {
		content += *it;
		it++;
		size -= 1;
	}
	frame.content = content;
	return frame;
}

void id3::set_frame(const id3::frame& frame, std::string& buf) {
	std::string strframe;
	auto header = get_id3_header(buf);

	strframe += frame.frame_id;
	auto encoded_size = id3::encode<4>(static_cast<uint64_t>(frame.size));
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

void id3::print_frame(const id3::frame& frame) {
	std::cout << frame.frame_id << ": ";
	std::cout << frame.content << std::endl; 
}

id3::frame id3::make_frame(const std::string& id, const std::string& cont, const std::string& flags) {
	uint32_t size = 4 + 2 + cont.length();
	id3::frame frame;
	frame.frame_id= id;
	frame.flags = flags;
	frame.content = cont;
	frame.size = size;
	return frame;
}	