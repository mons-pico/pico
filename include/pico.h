#ifndef PICO_H
#define PICO_H

/**
 * @file
 * Public API definitions for CPico.
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

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <pico_defs.h>
#include <pico_errno.h>


//======================================================================
// Debugging.
//======================================================================

/**
 * This is the global debugging flag.  Right now there are two settings:
 * 0 suppresses, and 1 enables.  The default is zero.
 */
extern int debug;

//#define TOKENPASTE(x, y) x ## y
//#define TOKENPASTE2(x, y) TOKENPASTE(x, y)

/**
 * Macro to output a debug message.  This works like `printf`.
 * @param m_fmt         The format string which must be a string literal.
 */
#define DEBUG(m_fmt, ...) \
    fprintf(stderr, "DEBUG (%s:%d): " m_fmt "\n", \
        __FILE__, __LINE__, ## __VA_ARGS__);

/**
 * Return the build date and time.  The returned string is an internal
 * static string, and should not be deallocated.
 *
 * @return      A string containing the date and time of the build.
 */
char * pico_build();

//======================================================================
// Memory handling.
//======================================================================

/**
 * Deallocate the given memory.  Internally the Pico library may use a
 * non-standard malloc / free, so it is important that data allocated by the
 * library be freed using this method.  DO NOT USE FREE!
 */
void pico_free(void * ptr);

//======================================================================
// Pico header information.
//======================================================================

/**
 * Different ways to represent the header when using `pico_dump_header`.
 */
typedef enum {
    PYTHON_DICT,    ///< Python dictionary literal.
    JSON,           ///< Strict JSON.
    YAML,           ///< Strict YAML.
    XML,            ///< Verbose XML.
} header_format_t;

/**
 * Obtain the Pico magic number.  The return value is always 0x91C0.
 *
 * @return          The Pico magic number.
 */
uint16_t pico_magic();

/**
 * Get the major version for this library.  Pico uses semantic versioning, so
 * any file with the same major version number must be compatible.
 *
 * @return          The Pico major version.
 */
uint16_t pico_major();

/**
 * Get the minor version for this library.  Pico uses semantic versioning, so
 * any file with the same major version number must be compatible.
 *
 * @return          The Pico minor version.
 */
uint16_t pico_minor();

/**
 * Obtain and return the major version from an open Pico file.
 *
 * @param pico      The Pico data structure.
 * @return          The stored major version.
 * @see             pico_major
 */
uint16_t pico_get_major(PICO * pico);

/**
 * Obtain and return the minor version from an open Pico file.
 *
 * @param pico      The Pico data structure.
 * @return          The stored minor version.
 * @see             pico_minor
 */
uint16_t pico_get_minor(PICO * pico);

/**
 * Get the offset value.  This is the zero-based offset in the file of the
 * first byte of encoded data.
 *
 * @param pico      The Pico data structure.
 * @return          The offset.
 */
uint32_t pico_get_offset(PICO * pico);

/**
 * Get the hash.  If the file has not been modified, then this is valid.
 * If the file has been modified, then this will force the hash to be
 * both re-computed and written to the file header, which can be quite
 * costly.  Caution: If the structure is NULL, this will return NULL.
 *
 * @param pico      The Pico data structure.
 * @return          The 16 byte hash value.  Do not deallocate this.
 */
uint8_t * pico_get_hash(PICO * pico);

/**
 * Get the number of bytes in the key.  Keys cannot be zero length.
 *
 * @return          The length, in bytes, of the key.
 */
uint16_t pico_get_key_length(PICO * pico);

/**
 * Get the key.  Caution: If the structure is NULL, this will return
 * NULL.
 *
 * @param pico      The Pico data structure.
 * @return          The bytes of the key.  Do not deallocate this.
 */
uint8_t * pico_get_key(PICO * pico);

/**
 * Dump the content of the header to the specified output stream as
 * text.
 *
 * @param pico      The Pico data structure.
 * @param format    The format to use.
 * @param out       An output stream.
 */
void pico_dump_header(PICO * pico, header_format_t format, FILE * out);

//======================================================================
// Pico file handling.
//======================================================================

/**
 * Create a Pico-encoded file, opened empty.  If the file exists prior to this
 * call, it will be overwritten.
 *
 * @param file      The file to create.
 * @param keylength The number of bytes in the key.
 * @param key       The key.  This value is copied; caller should deallocate.
 * @param md_length Reserve the specified number of bytes for metadata.
 * @return          The Pico data structure.  Test for `NULL`.
 */
PICO * pico_new(FILE * file, uint16_t keylength, uint8_t * key,
                 uint32_t md_length);

/**
 * Open a Pico-encoded file.  If the file does not exist the returned value
 * will indicate an error.
 *
 * @param file      The file to open or create.
 * @return          The Pico data structure.  Test for `NULL`.
 */
PICO * pico_open(FILE * file);

/**
 * Flush any data to the file and deallocate the Pico data structure.
 * Do not use `pico_free` or you will get a memory leak.  This does not
 * close the underlying stream.
 *
 * Do not use the pico data structure after this; it is invalid.
 *
 * @param pico      The Pico data structure.
 * @return          The last error code generated.
 */
pico_errno pico_finish(PICO * pico);

//======================================================================
// Pico error handling.
//======================================================================

/**
 * Returns true if the given Pico file is in an error condition, and false
 * otherwise.  Always returns true if the Pico data structure is `NULL`.
 *
 * @param pico      The Pico data structure.
 * @return          True iff an error was detected, or the structure is
 *                  `NULL`.
 */
bool pico_is_error(PICO * pico);

/**
 * Get the current error state from the Pico data structure.
 *
 * @param pico      The Pico data structure.
 * @return          The current error value.
 */
pico_errno pico_get_errno(PICO * pico);

/**
 * Get a human-readable error string from the Pico data structure.
 *
 * @param pico      The Pico data structure.
 * @return          The human-readable string.  Do not deallocate this!
 */
char * pico_print_error(PICO * pico);

/**
 * Clear any error condition in the given Pico data structure.
 *
 * @param pico      The Pico data structure.
 */
void pico_clear_error(PICO * pico);

//======================================================================
// Pico metadata information.
//======================================================================

/**
 * Get the length of the metadata, in bytes.  This may be zero.
 *
 * @param pico      The Pico data structure.
 * @return          The length of the metadata section, in bytes.
 */
size_t pico_get_md_length(PICO * pico);

/**
 * Extract some portion of the metadata from the header.  Note that the
 * returned value is always allocated to the provided length, and will be
 * `NULL` exactly when the length is zero or allocation fails.  If the
 * length exceeds the metadata available at the given position, then the
 * returned buffer is padded with zeros.  This is not considered an error.
 *
 * The caller is responsible for deallocating the return value using
 * `pico_free`.
 *
 * @param pico      The Pico data structure.
 * @param position  The zero-based offset into the metadata.
 * @param length    The number of bytes to extract.
 * @return          The extracted metadata.
 */
uint8_t * pico_get_metadata(PICO * pico, uint32_t position, uint32_t length);

/**
 * Write metadata to the header.  If there is not enough room at the given
 * position then the value is silently truncated.  Truncation is not
 * considered an error.
 *
 * If the length is zero, nothing is done and false is returned.  Otherwise
 * if the metadata is NULL, then true is returned (because of "truncation").
 *
 * @param pico      The Pico data structure.
 * @param position  The zero-based offset at which the first byte is written.
 * @param length    The number of bytes to write.  Zero is allowed.
 * @param md        The metadata to write.
 * @return          True iff data was truncated, was NULL, or `pico` was NULL.
 */
bool pico_set_metadata(PICO * pico, uint32_t position, uint32_t length,
                        uint8_t * md);

//======================================================================
// Pico data access.
//======================================================================

/**
 * Read and decode a block of data.  If there is not sufficient data at the
 * given position then the returned value is padded with nuls.
 *
 * If the length is zero or allocation fails, then 0 is returned.  If the
 * buffer is NULL then 0 is returned.
 *
 * The caller is responsible for deallocating the return value using
 * `pico_free`.
 *
 * @param pico      The Pico data structure.
 * @param position  The zero-based offset into the data.
 * @param length    The number of bytes to read.
 * @param data      A buffer to get the data.
 * @return          The number bytes read.
 */
size_t pico_get(PICO * pico, size_t position, size_t length, uint8_t * data);

/**
 * Encode and write the given block of data.  The return value is the actual
 * number of bytes successfully written.  This will always be the same as
 * length unless an error occurs.
 *
 * @param pico      The Pico data structure.
 * @param position  The zero-based offset into the data.
 * @param length    The number of bytes to write.
 * @param data      The data to write.
 * @return          The number of bytes written.
 */
size_t pico_set(PICO * pico, size_t position, size_t length, uint8_t * data);

//======================================================================
// Pico whole file operations.
//======================================================================

/**
 * Encode an entire file.
 * @param infile    The input file name.
 * @param outfile   The output file name.
 * @param keylen    Number of bytes in the key.
 * @param key       The key.
 * @param md_length Number of bytes to reserve for metadata.
 * @param err       A stream to get error messages.  If NULL, suppressed.
 * @return          The last error code.
 */
pico_errno encode_file(char * infile, char * outfile,
                        keylen_t keylen, uint8_t * key,
                        offset_t md_length, FILE * err);

/**
 * Decode an entire file.
 * @param infile    The input file name.
 * @param outfile   The output file name.
 * @param header    If true, write header information to standard out.
 * @param err       A stream to get error messages.  If NULL, suppressed.
 * @return          The last error code.
 */
pico_errno decode_file(char * infile, char * outfile, bool header, FILE * err);

#endif //PICO_H
