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
		std::copy(std::begin(sig), std::end(sig), std::begin(header.sig));
		header.ver_major = buf[3];
		header.ver_revision = buf[4];
		header.flags = buf[5];
		header.size = id3::decode(buf.substr(6,4));
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
		exheader.size = id3::decode(buf.substr(10, 4));
		exheader.number_of_flag_bytes = buf[14]; 
		exheader.flags = buf[15];

		if(exheader.flags[1])
			exheader.is_an_update = true;
		else
			exheader.is_an_update = false;
		
		if(exheader.flags[2]) {
			exheader.is_crc_data = true;
			id3::decode(buf.substr(16,5));
		} else {
			exheader.is_crc_data = false;
		}
		if(exheader.flags[3]) {
			auto i = exheader.is_crc_data ? 5 : 0;
			exheader.is_tag_restrictions = true;
			std::bitset<8> rests = buf[16+i];
			exheader.rests.tag_size = std::bitset<2>(rests.to_string().substr(0, 2)).to_ulong();
			exheader.rests.text_encoding = rests[2];
			exheader.rests.text_field_size = std::bitset<2>(rests.to_string().substr(3, 2)).to_ulong();
			exheader.rests.image_encoding = rests[5];
			exheader.rests.image_size = std::bitset<2>(rests.to_string().substr(6, 2)).to_ulong();
		} else {
			exheader.is_tag_restrictions = false;
		}
		return exheader;
	}
}

id3::footer id3::get_footer_from_header(const id3::header& header) {	
	id3::footer footer;
	std::string sig = "3DI";
	std::copy(sig.begin(), sig.end(), std::begin(footer.sig));
	footer.ver_major = header.ver_major;
	footer.ver_revision = header.ver_revision;
	footer.flags = header.flags;
	footer.size = header.size;
	return footer;
}

void id3::set_footer(const id3::footer& footer, std::string& buf) {
	buf += "3DI";
	buf += static_cast<char>(footer.ver_major);
	buf += static_cast<char>(footer.ver_revision);
	buf += footer.flags.to_string();
	auto encoded_size = id3::encode<4>(static_cast<uint64_t>(footer.size));
	std::string size(encoded_size.begin(), encoded_size.end());
	buf += size;
}	