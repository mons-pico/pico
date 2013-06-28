/**
 *        _
 *   _ __(_)__ ___
 *  | '_ \ / _/ _ \
 *  | .__/_\__\___/
 *  |_|            Pico
 *
 * @file
 * header_test.cpp
 *
 * @author ysp
 * @created Jan 6, 2013
 *
 * @section LICENSE
 * This program is Copyright (c) by UT-Battelle, LLC.
 * All rights reserved.
 */

#include <sstream>
#include "test_frame.h"
#include "io/header.h"

using std::string;

START_TEST

static char data[] = {
	0x91, 0xc0,                                     // magic
	0x00, 0x00,                                     // major
	0x00, 0x00,                                     // minor
	0x00, 0x00, 0x00, 0x29,                         // offset
	0x9f, 0x20, 0x7f, 0x81, 0x09, 0xbe, 0xe5, 0x4d, // hash
	0x7f, 0xc9, 0xd4, 0x04, 0xd6, 0xdf, 0xca, 0x20, // hash
	0x00, 0x0d,                                     // keylength
	0x00, 0x11, 0x77, 0x55, 0xff, 0xa8, 0x23, 0x26, // key
	0xa4, 0x3e, 0x2a, 0x7a, 0x43,                   // key
	0x54, 0x79, 0x1e, 0x26, 0xdf, 0xc1, 0x50, 0x06, // data
	0xc5, 0x1e, 0x7e, 0x3f, 0x10, 0x54, 0x3f, 0x7d, // data
	0x55, 0x00, 0xfd, 0x89, 0x67, 0x84, 0x4a, 0x4f, // data
	0x09, 0x37, 0x20, 0x65, 0x1f, 0x3c, 0x8c, 0x88, // data
	0x4a, 0x55, 0x8a, 0x09,                         // data
};
static uint16_t major = 0x0000;
static uint16_t minor = 0x0000;
static uint16_t keylength = 0x000d;
static uint32_t offset = 0x00000029;
static char hash[] = {
	0x9f, 0x20, 0x7f, 0x81, 0x09, 0xbe, 0xe5, 0x4d, // hash
	0x7f, 0xc9, 0xd4, 0x04, 0xd6, 0xdf, 0xca, 0x20, // hash
};
static char key[] = {
	0x00, 0x11, 0x77, 0x55, 0xff, 0xa8, 0x23, 0x26, // key
	0xa4, 0x3e, 0x2a, 0x7a, 0x43,                   // key
};
static string file("file_tmp");
static string file2("file2_tmp");
static pico::pico_hdr *hdr;

START_ITEM(create)
	// Write a Pico-encoded file.
	HANG("Creating a file");
	std::ofstream output;
	output.open(file.c_str());
	output.write(data, sizeof(data));
	output.close();
END_ITEM(create)

START_ITEM(read)
	// Read the header from a file.
	HANG("Reading the header")
	std::ifstream input;
	input.open(file.c_str());
	std::auto_ptr<pico::pico_hdr> header = pico::read_header(input);
	input.close();
	hdr = header.get();
	SUCCESS
	dump(*hdr);
END_ITEM(read)

START_ITEM(write)
	// Writing the header to a file.
	HANG("Writing the header")
	std::ofstream output;
	output.open(file2.c_str());
	pico::write_header(output, *hdr);
	output.flush();
	output.close();
END_ITEM(write)

START_ITEM(check)
	// Read the header from the second file.
	HANG("Reading the header")
	std::ifstream input;
	input.open(file2.c_str());
	std::auto_ptr<pico::pico_hdr> header = pico::read_header(input);
	input.close();
	SUCCESS
	dump(*hdr);
	HANG("Checking the header")
	char *ch1 = reinterpret_cast<char*>(hdr);
	char *ch2 = reinterpret_cast<char*>(header.get());
	for (int index = 0; index < sizeof(pico::pico_hdr); ++index) {
		if (ch1[index] != ch2[index]) {
			FAIL_ITEM(check, "mismatch at position " << index);
		}
	} // Compare.
END_ITEM(check)

START_ITEM(cleanup)
//	pico::delete_directory(file, true, false);
//	pico::delete_directory(file2, true, false);
END_ITEM(cleanup)

END_TEST
