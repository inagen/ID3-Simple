#pragma once
#include <string>
#include <bitset>
#include <stdint.h>
#include <iostream>
#include <array>

namespace id3 {

struct header {
    char sig[3];  			// "ID3" file identifier
	uint8_t ver_major;		// First byte of ID3v2 version
	uint8_t ver_revision;	// Second byte of ID3v2 version
	std::bitset<8> flags; 	// Flags in %abcd0000 format
	uint32_t size; 			// Size
};

struct footer {
    char sig[3];  			// "3DI" file identifier
	uint8_t ver_major;		// First byte of ID3v2 version
	uint8_t ver_revision;	// Second byte of ID3v2 version
	std::bitset<8> flags; 	// Flags in %abcd0000 format
	uint32_t size; 			// Size
};

struct tag_restrictions {
	uint8_t tag_size;
	bool text_encoding; 	
	uint8_t text_field_size;
	bool image_encoding;
	uint8_t image_size;
};

struct extended_header {
	uint32_t size;
	uint8_t number_of_flag_bytes;		
	std::bitset<8> flags;
	bool is_an_update;
	bool is_crc_data;
	uint8_t crc_data;
	bool is_tag_restrictions;
	tag_restrictions rests;	
};


std::string read_file(const std::string&);
bool write_file(const std::string&, const std::string&);

header get_id3_header(const std::string&);
void set_header(const header&, std::string&);

extended_header get_id3_extended_header(const std::string&);

footer get_footer_from_header(const header&);
void set_footer(footer&, std::string&);


uint64_t decode(const std::string&);

template<unsigned N>
std::array<uint8_t, N> encode(uint64_t value) {
    uint64_t result = 0;
    for (unsigned i = N-1; i < N; --i) {
        result |= (value & (0x7f << (i*7))) << i;
    }
    std::array<uint8_t, N> data = {};
    for (unsigned i = N-1; i < N; --i) {
        data[i] = result & (0xff << (i*8));
    }
    return data;
}

}