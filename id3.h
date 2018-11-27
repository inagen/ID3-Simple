#pragma once
#include <string>
#include <bitset>
#include <stdint.h>
#include <vector>
#include <iostream>

namespace id3 {

struct header {
    char sig[3];  			// "ID3" file identifier
	uint8_t ver_major;		// First byte of ID3v2 version
	uint8_t ver_revision;	// Second byte of ID3v2 version
	std::bitset<8> flags; 	// Flags in %abcd0000 format
	uint32_t size; 			// Size
};


std::string read_file(const std::string&);
bool write_file(const std::string&, const std::string&);
header get_id3_header(const std::string&);


template<size_t N>
uint32_t size_to_uint32(std::bitset<N> buf) {
	auto res = buf.to_string();
	int k = 0;
	for(int i = 0; i < N; i+=8) {
		res.erase(res.begin()+i-k);
		k++;  
	}
	
	std::bitset<N/8*7>  bitset(res);
	return static_cast<uint32_t>(bitset.to_ulong());
}

}