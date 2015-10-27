#ifndef PICO_HEADER_H
#define PICO_HEADER_H

/**
 * @file
 * Pico header format and definitions used in pico.
 *
 * @code{text}
 *       _
 *  _ __(_)__ ___
 * | '_ \ / _/ _ \
 * | .__/_\__\___/
 * |_|            Pico
 * @endcode
 *
 * Copyright (c) 2015 by Stacy Prowell, all rights reserved.  Licensed under
 * the BSD 2-Clause license.  See the file license that is part of this
 * distribution.  This file may not be copied, modified, or distributed except
 * according to those terms.
 */

#ifdef __linux__
#include <arpa/inet.h>
#endif //__linux__
#include "pico_errno.h"

/// Pico file magic number.
#define MAGIC (0x91c0)
/// Major version number of supported Pico format.
#define VERSION_MAJOR 1
/// Minor version number of supported Pico format.
#define VERSION_MINOR 0

// Define the header types.  This is the only place you should
// need to worry with the size of header items other than the
// hash.

/// Type of magic number.
typedef uint16_t magic_t;
/// Type of major version number.
typedef uint16_t major_t;
/// Type of minor version number.
typedef uint16_t minor_t;
/// Type for header offset.
typedef uint32_t offset_t;
/// Type for key length in header.
typedef uint16_t keylen_t;

// Define header offsets.  Note that these are all compile-time
// constants.  The header must fit into a 32-bit address space.

/// Offset to magic number in header.
#define MAGIC_POS 0
/// Offset to major version in header.
#define MAJOR_POS (MAGIC_POS + (uint32_t)sizeof(magic_t))
/// Offset to minor version in header.
#define MINOR_POS (MAJOR_POS + (uint32_t)sizeof(major_t))
/// Offset to the data offset in the header.
#define OFFSET_POS (MINOR_POS + (uint32_t)sizeof(minor_t))
/// Offset to the hash in the header.
#define HASH_POS (OFFSET_POS + (uint32_t)sizeof(offset_t))
/// Length of hash in header.
#define HASH_LEN 16
/// Offset to key length in header.
#define KEYLEN_POS (HASH_POS + HASH_LEN)
/// Offset to key in header.
#define KEY_POS (KEYLEN_POS + (uint32_t)sizeof(keylen_t))

// Re-package htonl, htons, ntohl, and ntohs to hide size issues.

/// Convert a 16- or 32-bit number from network order to host order.
#define NTOH(m_type, m_item) \
    (m_type)(sizeof(m_type) == 2 ? ntohs(m_item) : ntohl(m_item))

/// Convert a 16- or 32-bit number from host order to network order.
#define HTON(m_type, m_item) \
    (m_type)(sizeof(m_type) == 2 ? htons(m_item) : htonl(m_item))

/**
 * The Pico data structure.
 */
typedef struct {

    FILE * file;                ///< Backing file stream.
    major_t major;              ///< Version in header.
    minor_t minor;              ///< Version in header.
    offset_t offset;            ///< Zero-based offset to data.
    uint8_t hash[HASH_LEN];     ///< Hash.
    bool hash_valid;            ///< Whether the hash is valid.
    keylen_t key_length;        ///< Key length.
    uint8_t * key;              ///< Key.
    pico_errno errno;           ///< Error code of last operation.
    char error_text[1024];      ///< Error text.
    uint32_t md_length;         ///< Reserved metadata length.
    size_t data_length;         ///< Data length (unused).

} PICO;

// The chunk size to use when reading and writing.  This should correspond to
// a page size on your machine.  You can \#define this prior to including this
// file to override.
#ifndef CHUNK_SIZE
/// Size of chunk to use.
#define CHUNK_SIZE 4096
#endif

#endif //PICO_HEADER_H
