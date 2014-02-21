#ifndef HEADER_H_
#define HEADER_H_

/**
 *        _
 *   _ __(_)__ ___
 *  | '_ \ / _/ _ \
 *  | .__/_\__\___/
 *  |_|            Pico
 *
 * @file
 * Define the structure of a Pico encoded file's header.
 *
 * @author ysp
 * @created Jan 5, 2013
 *
 * @section LICENSE
 * This program is Copyright (c) by UT-Battelle, LLC.
 * All rights reserved.
 */


#include <stdint.h>
#include <openssl/md5.h>
#include <iostream>
#include <vector>

namespace pico
{

/// The magic string used in the header.
static const char MAGIC[] = { 0x91, 0xc0 };

/**
 * Encapsulate the header for a Pico-encoded file.
 *
 * To use this make an instance using one of the static methods.
 */
class Header
{
public:
	/// Offset to the magic string.
	static const off_t MAGIC_OFFSET = 0;
	/// Length of the magic string.
	static const size_t MAGIC_LENGTH = 2;
	/// Offset to major version number.
	static const off_t MAJOR_OFFSET = MAGIC_OFFSET + MAGIC_LENGTH;
	/// Length of major version number.
	static const size_t MAJOR_LENGTH = sizeof(uint16_t);
	/// Offset to minor version number.
	static const off_t MINOR_OFFSET = MAJOR_OFFSET + MAJOR_LENGTH;
	/// Length of minor version number.
	static const size_t MINOR_LENGTH = sizeof(uint16_t);
	/// Offset to data offset.
	static const off_t OFFSET_OFFSET = MINOR_OFFSET + MINOR_LENGTH;
	/// Length of data offset.
	static const size_t OFFSET_LENGTH = sizeof(uint16_t);
	/// Offset to hash.
	static const off_t HASH_OFFSET = OFFSET_OFFSET + OFFSET_LENGTH;
	/// Length of hash.
	static const size_t HASH_LENGTH = MD5_DIGEST_LENGTH;
	/// Offset to key length.
	static const off_t KEYLENGTH_OFFSET = HASH_OFFSET + HASH_LENGTH;
	/// Length of key length.
	static const size_t KEYLENGTH_LENGTH = sizeof(uint16_t);
	/// Offset to key.
	static const off_t KEY_OFFSET = KEYLENGTH_OFFSET + KEYLENGTH_LENGTH;

private:
	/// The fixed part of the header.
	std::vector<char> hdr;

	/// The key.
	std::vector<char> *key;

	/**
	 * Construct a new header.
	 */
	Header(): hdr(KEY_OFFSET - MAGIC_OFFSET) {
		// The key will be read from the header.
		key = 0L;
	};

	/**
	 * Dispose of a header.
	 */
	~Header() { delete(key); };


};



/**
 * Define the complete Pico header.  Caution: This struct cannot simply be
 * written on every platform, because the C++ standard does not guarantee the
 * memory layout.
 */
struct pico_hdr {
	uint8_t magic[2];
	uint16_t major;
	uint16_t minor;
	uint32_t offset;
	uint8_t hash[Header::HASH_LENGTH];
	uint16_t keylength;
	uint8_t* key;
	pico_hdr() : major(0), minor(0), offset(0L), keylength(0), key(0L) {
		magic[0] = 0x91; magic[1] = 0xc0;
	}
	~pico_hdr() { delete(key); }
};

/**
 * Read the header from the given stream.  If the header is incorrect, an
 * invalid_argument exception is thrown.
 *
 * @param input		The input stream.
 * @return			The header.
 * @throws			std::invalid_argument
 * 					The header is not correctly formatted, the file is not
 * 					a Pico encoded file, or the version cannot be read by this
 * 					software.
 */
std::auto_ptr<pico_hdr> read_header(std::istream& input);

/**
 * Write the header to the given stream.  If the write fails, then an
 * exception is thrown.
 *
 * @param output	The output stream.
 * @param header	The header to write.
 * @throws			std::invalid_argument
 */
void write_header(std::ostream& output, const pico_hdr& header);

/**
 * Dump information about the header to the standard output pointed to by
 * std::cout.
 *
 * @param header	The pico header.
 */
void dump(const pico_hdr& header);

}


#endif /* HEADER_H_ */
