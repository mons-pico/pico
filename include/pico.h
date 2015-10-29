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
extern int pico_debug;

//#define TOKENPASTE(x, y) x ## y
//#define TOKENPASTE2(x, y) TOKENPASTE(x, y)

/**
 * Macro to output a debug message.  This works like `printf`.
 * @param m_fmt         The format string which must be a string literal.
 */
#define DEBUG(m_fmt, ...) \
    if (pico_debug) fprintf(stderr, "DEBUG (%s:%d): " m_fmt "\n", \
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
 * If the provided structure is NULL, then 0 is returned.
 *
 * @param pico      The Pico data structure.
 * @return          The stored major version.
 * @see             pico_major
 */
uint16_t pico_get_major(PICO * pico);

/**
 * Obtain and return the minor version from an open Pico file.
 *
 * If the provided structure is NULL, then zero is returned.
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
 * If the provided structure is NULL, then zero is returned.
 *
 * @param pico      The Pico data structure.
 * @return          The offset.
 */
uint32_t pico_get_offset(PICO * pico);

/**
 * Get the hash.  If the file has not been modified, then this is valid.
 * If the file has been modified, then this will force the hash to be
 * both re-computed and written to the file header, which can be quite
 * costly.
 *
 * If the provided structure is NULL, then NULL is returned.
 *
 * @param pico      The Pico data structure.
 * @return          The 16 byte hash value.  Do not deallocate this.
 */
uint8_t * pico_get_hash(PICO * pico);

/**
 * Get the number of bytes in the key.  Keys cannot be zero length.
 *
 * If the provided structure is NULL, then zero is returned.
 *
 * @return          The length, in bytes, of the key.
 */
uint16_t pico_get_key_length(PICO * pico);

/**
 * Get the key.
 *
 * If the provided structure is NULL, then NULL is returned.
 *
 * @param pico      The Pico data structure.
 * @return          The bytes of the key.  Do not deallocate this.
 */
uint8_t * pico_get_key(PICO * pico);

/**
 * Dump the content of the header to the specified output stream as
 * text.
 *
 * If either the structure or file is NULL, then nothing is done.
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
 * If the file is NULL, the key length is less than one, or the key is NULL,
 * then NULL is returned.  If an error is detected and the structure cannot
 * be constructed, then NULL is returned.
 *
 * @param file      The file to create.
 * @param keylength The number of bytes in the key.
 * @param key       The key.  This value is copied; caller should deallocate.
 * @param md_length Reserve the specified number of bytes for metadata.
 * @param perrno    If an error occurs and this is not NULL, it holds the
 *                  error condition code.
 * @return          The Pico data structure.  Test for `NULL`.
 */
PICO * pico_new(FILE * file, uint16_t keylength, uint8_t * key,
                 uint32_t md_length, pico_errno * perrno);

/**
 * Open a Pico-encoded file.
 *
 * If the file is NULL or an error occurs in processing (corrupt header,
 * not a Pico file, no memory) then NULL is returned.
 *
 * @param file      The file to open or create.
 * @param perrno    If an error occurs and this is not NULL, it holds the
 *                  error condition code.
 * @return          The Pico data structure.  Test for `NULL`.
 */
PICO * pico_open(FILE * file, pico_errno * perrno);

/**
 * Flush any data to the file and deallocate the Pico data structure.
 * Do not use `pico_free` or you will get a memory leak.  This does not
 * close the underlying stream.
 *
 * Do not use the pico data structure after this; it is invalid.  If the
 * provided structure is NULL, nothing is done and OK is returned.
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
 * otherwise.
 *
 * Always returns true if the Pico data structure is `NULL`.
 *
 * @param pico      The Pico data structure.
 * @return          True iff an error was detected, or the structure is
 *                  `NULL`.
 */
bool pico_is_error(PICO * pico);

/**
 * Get the current error state from the Pico data structure.
 *
 * Always returns ISNULL if the structure is NULL.
 *
 * @param pico      The Pico data structure.
 * @return          The current error value.
 */
pico_errno pico_get_errno(PICO * pico);

/**
 * Get a human-readable error string from the Pico data structure.
 *
 * Returns "Null pointer." iff the data structure is NULL.
 *
 * @param pico      The Pico data structure.
 * @return          The human-readable string.  Do not deallocate this!
 */
char * pico_print_error(PICO * pico);

/**
 * Clear any error condition in the given Pico data structure.
 *
 * Ignored if the structure is NULL.
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
 * If the structure is NULL this returns zero.
 *
 * @param pico      The Pico data structure.
 * @return          The length of the metadata section, in bytes.
 */
size_t pico_get_md_length(PICO * pico);

/**
 * Extract some portion of the metadata from the header.  If there is not
 * enough metadata at the given position to return, or if the position is past
 * the end of the metadata, then the buffer is padded with nuls.  This is not
 * considered an error.
 *
 * If either the structure or data is NULL then nothing is done and zero is
 * returned.
 *
 * @param pico      The Pico data structure.
 * @param position  The zero-based offset into the metadata.
 * @param length    The number of bytes to extract.
 * @param data      A buffer to get the extracted metadata.
 * @return          The number of bytes of metadata actually extracted.
 */
uint32_t pico_get_metadata(PICO * pico, uint32_t position, uint32_t length,
                           uint8_t * data);

/**
 * Write metadata to the header.  If there is not enough room at the given
 * position then the value is silently truncated.  Truncation is not
 * considered an error.
 *
 * If either the structure or data is NULL then nothing is done and zero is
 * returned.
 *
 * @param pico      The Pico data structure.
 * @param position  The zero-based offset at which the first byte is written.
 * @param length    The number of bytes to write.  Zero is allowed.
 * @param md        The metadata to write.
 * @return          The number of bytes of metadata actually set.
 */
uint32_t pico_set_metadata(PICO * pico, uint32_t position, uint32_t length,
                           uint8_t * md);

//======================================================================
// Pico data access.
//======================================================================

/**
 * Read and decode a block of data.  If there is not sufficient data at the
 * given position then the returned value is padded with nuls.  This is not
 * considered an error.
 *
 * If either the structure or data is NULL then nothing is done and zero is
 * returned.
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
 * If either the structure or data is NULL then nothing is done and zero is
 * returned.
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
 *
 * @param infile    The input file name.
 * @param outfile   The output file name.
 * @param keylen    Number of bytes in the key.
 * @param key       The key.
 * @param md_length Number of bytes to reserve for metadata.
 * @param err       A stream to get error messages.  If NULL, suppressed.
 * @return          The last error code.
 */
pico_errno pico_encode_file(char *infile, char *outfile,
                            keylen_t keylen, uint8_t *key,
                            offset_t md_length, FILE *err);

/**
 * Decode an entire file.
 *
 * @param infile    The input file name.
 * @param outfile   The output file name.
 * @param testhash  Iff true, compute and check the hash during decode as a CRC.
 * @param err       A stream to get error messages.  If NULL, suppressed.
 * @return          The last error code.
 */
pico_errno pico_decode_file(char *infile, char *outfile, bool testhash,
                            FILE *err);

#endif //PICO_H
