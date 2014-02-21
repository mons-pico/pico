/**
 *        _
 *   _ __(_)__ ___
 *  | '_ \ / _/ _ \
 *  | .__/_\__\___/
 *  |_|            Pico
 *
 * @file
 * header.cpp
 *
 * @author ysp
 * @created Jan 6, 2013
 *
 * @section LICENSE
 * This program is Copyright (c) by UT-Battelle, LLC.
 * All rights reserved.
 */

#include "header.h"
#include <arpa/inet.h>
#include <iomanip>

using namespace std;

namespace pico {

auto_ptr<pico_hdr> read_header(istream& input) {
	// Read the fixed part of the header directly from the stream.  We have to
	// deal with endianness problems.
	pico_hdr* hdr(new pico_hdr);
	// Harder case.  We must read each integer and then "fix it" prior to
	// storing it.  Note that Pico uses network byte order, so we can use
	// the POSIX functions to convert.
	uint16_t major, minor, keylength;
	uint32_t offset;
	input.read(reinterpret_cast<char*>(hdr->magic), sizeof(hdr->magic));
	input.read(reinterpret_cast<char*>(&major), sizeof(major));
	hdr->major = ntohs(major);
	input.read(reinterpret_cast<char*>(&minor), sizeof(minor));
	hdr->minor = ntohs(minor);
	input.read(reinterpret_cast<char*>(&offset), sizeof(offset));
	hdr->offset = ntohl(offset);
	input.read(reinterpret_cast<char*>(hdr->hash), Header::HASH_LENGTH);
	input.read(reinterpret_cast<char*>(&keylength), sizeof(keylength));
	hdr->keylength = ntohs(keylength);
	// Now read the key.
	hdr->key = new uint8_t[hdr->keylength];
	input.read(reinterpret_cast<char*>(hdr->key), keylength);
	// Seek to the offset and return the result.
	input.seekg(hdr->offset);
	return auto_ptr<pico_hdr>(hdr);
}

#define SEND_S(item_m) { \
	uint16_t tmp = htons(item_m); \
	output.write(reinterpret_cast<const char*>(&tmp), sizeof(uint16_t)); \
}
#define SEND_L(item_m) { \
	uint32_t tmp = htonl(item_m); \
	output.write(reinterpret_cast<const char*>(&tmp), sizeof(uint32_t)); \
}

void write_header(ostream& output, const pico_hdr& hdr) {
	// Harder case.  We must write each integer after "fixing" it.  Note
	// that Pico uses network byte order, so we can use the POSIX functions
	// to convert.
	output.write(reinterpret_cast<const char*>(hdr.magic), Header::MAGIC_LENGTH);
	SEND_S(hdr.major);
	SEND_S(hdr.minor);
	SEND_L(hdr.offset);
	output.write(reinterpret_cast<const char*>(hdr.hash), Header::HASH_LENGTH);
	SEND_S(hdr.keylength);
	output.write(reinterpret_cast<const char*>(hdr.key), hdr.keylength);
	output.flush();
}

void dump(const pico_hdr& hdr) {
	cout << "     magic: [ 0x"
			<< setbase(16) << setfill('0') << setw(2)
			<< (unsigned int) hdr.magic[0]
			<< ", 0x" << setw(2)
			<< (unsigned int) hdr.magic[1] << " ]" << endl;
	cout << "   version: "
			<< setbase(10) << setw(0)
			<< hdr.major << '.' << hdr.minor << endl;
	cout << "    offset: 0x"
			<< setbase(16) << setw(8) << hdr.offset << endl;
	cout << "      hash: [";
	for (int index = 0; index < Header::HASH_LENGTH; ++index) {
		cout << " 0x" << setw(2) << (unsigned int) hdr.hash[index];
	} // Write the hash.
	cout << " ]" << endl;
	cout << "key length: "
			<< setbase(10) << setw(0) << hdr.keylength << endl;
	cout << "       key: [" << setbase(16);
	for (int index = 0; index < hdr.keylength; ++index) {
		cout << " 0x" << setw(2) << (unsigned int) hdr.key[index];
	} // Write the key.
	cout << " ]" << endl;
}

}
