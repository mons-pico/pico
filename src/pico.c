/**
 * @file
 * Implementation and private API for CPico.
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
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include <pico.h>
#include "md5.h"


/**
 * The debugging level.  Right now there are two levels: 0 (suppress) and 1
 * (emit debugging information).
 */
int pico_debug = 0;

/**
 * Print an error message.  This works like fprintf.
 * @param m_err     The stream.  If NULL, suppress the message.
 * @param m_fmt     The format string.
 */
#define ERR(m_err, m_fmt, ...) \
    if ((m_err) != NULL) fprintf((m_err), (m_fmt), ##__VA_ARGS__)

/// Local magic number, so we can reference it by pointer.
static magic_t magic = MAGIC;

// Define memory handling.  Feel free to override this with jeMalloc,
// nedmalloc, or something else.  Note that to do this you simply
// need to include the appropriate header and then \#define MALLOC
// and FREE before you include this file.

#ifndef MALLOC
/**
 * Allocate memory.
 * @param m_thing     The thing to allocate.  A pointer to this type is
 *                    returned.
 * @param m_number    How many things to allocate.
 * @return            A pointer to the allocated things.
 */
#define MALLOC(m_thing, m_number) (m_thing *)calloc(m_number, sizeof(m_thing))
#endif
#ifndef FREE
/**
 * Free a pointer.  NULL pointers are ignored.
 *
 * @param m_ptr       Pointer to the thing to deallocate.
 */
#define FREE(m_ptr) ((m_ptr == NULL) ? NULL : free(m_ptr), NULL)
#endif

void
pico_free(void * ptr) {
    FREE(ptr);
}

/**
 * Print a region of memory as comma-separated hex values.
 *
 * @param thing     Pointer to memory.
 * @param length    Number of bytes to print.
 * @param hex       If true, print hex.  Otherwise, print decimal.
 * @param out       Stream to get output.
 */
static void
print_array(void *thing, size_t length, bool hex, FILE *out) {
    bool first = true;
    for (size_t index = 0; index < length; ++index) {
        if (first) {
            first = false;
        } else {
            fprintf(out, ", ");
        }
        if (hex) {
            fprintf(out, "0x%02X", *((uint8_t *) (thing + index)));
        } else {
            fprintf(out, "%d", *((uint8_t *) (thing + index)));
        }
    } // Write all bytes.
}

/**
 * Print a region of memory as a string of hex digits.
 *
 * @param thing     Pointer to memory.
 * @param length    Number of bytes to print.
 * @param out       Stream to get output.
 */
static void
print_hexstring(void *thing, size_t length, FILE *out) {
    bool first = true;
    for (size_t index = 0; index < length; ++index) {
        fprintf(out, "%02X", *((uint8_t *)(thing + index)));
    } // Write all bytes.
}

/**
 * Print an integer.  Integers of size 1, 2, 4, and 8 bytes are
 * accepted.
 *
 * @param thing     Pointer to the integer.
 * @param length    Number of bytes in the integer.
 * @param hex       If true, print hex.  Otherwise, print decimal.
 * @param out       Stream to get the output.
 */
static void
print_int(void *thing, size_t length, bool hex, FILE *out) {
    if (hex) {
        if (length == 1) {
            fprintf(out, "0x%02" PRIX8, *((uint8_t *) (thing)));
        } else if (length == 2) {
            fprintf(out, "0x%04" PRIX16, *((uint16_t *) (thing)));
        } else if (length == 4) {
            fprintf(out, "0x%08" PRIX32, *((uint32_t *) (thing)));
        } else if (length == 8) {
            fprintf(out, "0x%16" PRIX64, *((uint64_t *) (thing)));
        }
    } else {
        if (length == 1) {
            fprintf(out, "%" PRId8, *((uint8_t *) (thing)));
        } else if (length == 2) {
            fprintf(out, "%" PRId16, *((uint16_t *) (thing)));
        } else if (length == 4) {
            fprintf(out, "%" PRId32, *((uint32_t *) (thing)));
        } else if (length == 8) {
            fprintf(out, "%" PRId64, *((uint64_t *) (thing)));
        }
    }
}

/**
 * Encrypt or decrypt the data in place using the provided key.
 * @param data         The data to encrypt or decrypt.
 * @param len          Length of the data array.
 * @param key          The encryption key.
 * @param keylen       The number of bytes in the key.
 * @param position     The position of the data in the overall data stream.
 * @return             The data.
 */
uint8_t * crypt(uint8_t * data, size_t len, uint8_t * key, keylen_t keylen,
                size_t position) {
    for (size_t index = 0; index < len; index++) {
        data[index] ^= key[(size_t)(index+position) % keylen];
    } /* Process all bytes. */
    return data;
}

char * pico_build() {
    return "" __DATE__ ", " __TIME__;
}

//======================================================================
// Pico header information.
//======================================================================

magic_t
pico_magic() {
    return magic;
}

major_t
pico_major() {
    return VERSION_MAJOR;
}

minor_t
pico_minor() {
    return VERSION_MINOR;
}

major_t
pico_get_major(PICO * pico) {
    if (pico != NULL) return pico->major; else return 0;
}

minor_t
pico_get_minor(PICO * pico) {
    if (pico != NULL) return pico->minor; else return 0;
}

offset_t
pico_get_offset(PICO * pico) {
    if (pico != NULL) return pico->offset; else return 0;
}

uint8_t *
pico_get_hash(PICO * pico) {
    if (pico == NULL) return NULL;

    // See if the hash is valid, or if we need to re-compute it.
    if (pico->hash_valid) return pico->hash;

    // We have to re-compute the hash.  To do this, read back
    // through the entire file and compute the hash.  Then store
    // it in the header and write the header back to the file.

    size_t position = 0;
    size_t bytes = 0;
    uint8_t * data = MALLOC(uint8_t, CHUNK_SIZE);
    if (data == NULL) {
        pico->errno = NO_MEMORY;
        sprintf(pico->error_text, "Cannot get memory for buffer.");
    }

    // Initialize the hash computation.
    MD5_CTX md5data;
    MD5_Init(&md5data);

    // Compute the hash.
    while ((bytes = pico_get(pico, position, CHUNK_SIZE, data)) > 0) {
        MD5_Update(&md5data, data, bytes);
        position += bytes;
    } // Compute the hash over all bytes.

    // Done.
    MD5_Final(pico->hash, &md5data);

    // The hash is now valid!
    pico->hash_valid = true;
    return pico->hash;
}

keylen_t
pico_get_key_length(PICO * pico) {
    if (pico != NULL) return pico->key_length; else return 0;
}

uint8_t *
pico_get_key(PICO * pico) {
    if (pico != NULL) return pico->key; else return NULL;
}

void
pico_dump_header(PICO * pico, header_format_t format, FILE * out) {
    if (pico == NULL) return;
    if (out == NULL) return;

    // Python dictionary and JSON are almost exactly the same.  Almost.
    // So is YAML.
    switch (format) {
        case PYTHON_DICT:
        case JSON:
            // Write the header information as either JSON or a Python dict.
            fprintf(out, "{\n");
            fprintf(out, "    \"magic\" : [ ");
            print_array(&magic, sizeof(magic_t), format == PYTHON_DICT, out);
            fprintf(out, " ],\n");

            fprintf(out, "    \"major\" : ");
            print_int(&pico->major, sizeof(major_t), false, out);
            fprintf(out, ",\n");

            fprintf(out, "    \"minor\" : ");
            print_int(&pico->minor, sizeof(minor_t), false, out);
            fprintf(out, ",\n");

            fprintf(out, "    \"offset\" : ");
            print_int(&pico->offset, sizeof(offset_t), format == PYTHON_DICT, out);
            fprintf(out, ",\n");

            fprintf(out, "    \"md5\" : \"");
            print_hexstring(pico->hash, HASH_LEN, out);
            fprintf(out, "\",\n");

            fprintf(out, "    \"key_length\" : ");
            print_int(&pico->key_length, sizeof(keylen_t), false, out);
            fprintf(out, ",\n");

            fprintf(out, "    \"key\" : [ ");
            print_array(pico->key, pico->key_length, format == PYTHON_DICT, out);
            fprintf(out, " ]\n");

            fprintf(out, "}\n");
            break;

        case YAML:
            // Write header as YAML.
            fprintf(out, "    magic : [ ");
            print_array(&magic, sizeof(magic_t), true, out);
            fprintf(out, " ]\n");

            fprintf(out, "    major : ");
            print_int(&pico->major, sizeof(major_t), false, out);
            fprintf(out, "\n");

            fprintf(out, "    minor : ");
            print_int(&pico->minor, sizeof(minor_t), false, out);
            fprintf(out, "\n");

            fprintf(out, "    offset : ");
            print_int(&pico->offset, sizeof(offset_t), true, out);
            fprintf(out, "\n");

            fprintf(out, "    md5 : ");
            print_hexstring(pico->hash, HASH_LEN, out);
            fprintf(out, "\n");

            fprintf(out, "    key_length : ");
            print_int(&pico->key_length, sizeof(keylen_t), false, out);
            fprintf(out, "\n");

            fprintf(out, "    key : [ ");
            print_array(pico->key, pico->key_length, true, out);
            fprintf(out, " ]\n");
            break;

        case XML:
            // Write header as XML.
            fprintf(out, "<pico magic='");
            print_hexstring(&magic, sizeof(magic_t), out);
            fprintf(out, "'\n");

            fprintf(out, "    major='");
            print_int(&pico->major, sizeof(major_t), false, out);
            fprintf(out, "'\n");

            fprintf(out, "    minor='");
            print_int(&pico->minor, sizeof(minor_t), false, out);
            fprintf(out, "'\n");

            fprintf(out, "    offset='");
            print_int(&pico->offset, sizeof(offset_t), false, out);
            fprintf(out, "'\n");

            fprintf(out, "    md5='");
            print_hexstring(pico->hash, HASH_LEN, out);
            fprintf(out, "'\n");

            fprintf(out, "    key_length='");
            print_int(&pico->key_length, sizeof(keylen_t), false, out);
            fprintf(out, "'\n");

            fprintf(out, "    key='");
            print_hexstring(pico->key, pico->key_length, out);
            fprintf(out, "' />\n");
            break;

        default:
            // Do nothing.
            break;
    }
}

//======================================================================
// Pico file handling.
//======================================================================

/**
 * Write the header to the start of the file.  This sets the error fields.
 * This does not necessarily compute the hash; we want to leave that until
 * the last moment, since it can be costly.
 *
 * @param pico          The Pico data structure.
 * @return              True iff there is an error.
 */
static bool
write_header(PICO * pico) {
    // Seek to the start of the file.
    if (fseek(pico->file, 0, SEEK_SET)) {
        pico->errno = CANNOT_SEEK;
        sprintf(pico->error_text, "Unable to seek to start of file.");
        return true;
    }

    // Allocate the header.
    uint8_t * header = MALLOC(uint8_t, KEY_POS + pico->key_length);
    if (header == NULL) {
        pico->errno = NO_MEMORY;
        sprintf(pico->error_text, "Cannot get memory to write header.");
        return true;
    }

    // Write the data into the header.
    *((magic_t *)(header + MAGIC_POS)) = HTON(magic_t, magic);
    *(major_t *)(header + MAJOR_POS) = HTON(major_t, pico->major);
    *(minor_t *)(header + MINOR_POS) = HTON(minor_t, pico->minor);
    *(offset_t *)(header + OFFSET_POS) = HTON(offset_t, pico->offset);
    memcpy(header + HASH_POS, pico->hash, HASH_LEN);
    *(keylen_t *)(header + KEYLEN_POS) = HTON(keylen_t, pico->key_length);
    memcpy(header + KEY_POS, pico->key, pico->key_length);

    // Write the header to the file.
    size_t size = fwrite(header, 1, KEY_POS + pico->key_length, pico->file);
    if (size != KEY_POS + pico->key_length) {
        pico->errno = CANNOT_WRITE;
        sprintf(pico->error_text, "Unable to write header to file.");
        return true;
    }

    // Success.
    pico->errno = OK;
    return false;
}

PICO *
pico_new(FILE * file, uint16_t keylength, uint8_t * key, uint32_t md_length) {
    // Allocate immediately.
    PICO * pico = MALLOC(PICO, 1);
    if (pico == NULL) return NULL;

    // Initialize parts of the header.
    pico->file = file;
    pico->major = VERSION_MAJOR;
    pico->minor = VERSION_MINOR;
    pico->offset = KEY_POS + keylength + md_length;
    pico->hash_valid = false;
    pico->errno = OK;
    pico->md_length = 0;
    pico->data_length = 0;

    // Save the key.
    pico->key_length = keylength;
    if (keylength > 0) {
        if (key == NULL) {
            pico->errno = KEY_ERROR;
            sprintf(pico->error_text, "Key data is null but length is not zero.");
            return pico;
        }
        pico->key = MALLOC(uint8_t, keylength);
        if (pico->key == NULL) {
            pico->errno = NO_MEMORY;
            sprintf(pico->error_text, "Cannot get memory for key.");
            return pico;
        }
        memcpy(pico->key, key, keylength);
    } else {
        // Always allocate for key so that it is not NULL.
        pico->key = MALLOC(uint8_t, 1);
        if (pico->key == NULL) {
            pico->errno = NO_MEMORY;
            sprintf(pico->error_text, "Cannot get memory for key.");
            return pico;
        }
    }

    // Write the header now.
    if (write_header(pico)) {
        return pico;
    }

    // Done.
    return pico;
}

/**
 * Read and validate a Pico header from a file.
 *
 * @param pico          The Pico data structure.  This must have the file.
 * @return              True iff there was an error.
 */
static bool
read_header(PICO * pico) {
    if (fseek(pico->file, 0, SEEK_SET)) {
        pico->errno = CANNOT_SEEK;
        sprintf(pico->error_text, "Unable to seek to start of file.");
        return true;
    }

    // Read and check the header.
    magic_t file_magic = 0;
    if (fread(&file_magic, 1, sizeof(magic_t), pico->file) <
            sizeof(magic_t)) {
        pico->errno = CANNOT_READ;
        sprintf(pico->error_text, "Cannot read magic number.");
        return true;
    }
    file_magic = NTOH(magic_t, file_magic);
    if (file_magic != magic) {
        pico->errno = NOT_PICO;
        sprintf(pico->error_text, "Incorrect magic number in file.");
        return true;
    }
    if (fread(&pico->major, 1, sizeof(major_t), pico->file) <
            sizeof(major_t)) {
        pico->errno = CANNOT_READ;
        sprintf(pico->error_text, "Cannot read major version number.");
        return true;
    }
    pico->major = NTOH(major_t, pico->major);
    if (pico->major != VERSION_MAJOR) {
        pico->errno = BAD_VERSION;
        sprintf(pico->error_text, "File has incompatible version %d.x.",
                pico->major);
        return true;
    }
    if (fread(&pico->minor, 1, sizeof(minor_t), pico->file) <
            sizeof(minor_t)) {
        pico->errno = CANNOT_READ;
        sprintf(pico->error_text, "Cannot read minor version number.");
        return true;
    }
    pico->minor = NTOH(minor_t, pico->minor);
    if (fread(&pico->offset, 1, sizeof(offset_t), pico->file) <
            sizeof(offset_t)) {
        pico->errno = CANNOT_READ;
        sprintf(pico->error_text, "Cannot read data offset.");
        return true;
    }
    pico->offset = NTOH(offset_t, pico->offset);
    if (fread(pico->hash, 1, HASH_LEN, pico->file) <
            HASH_LEN) {
        pico->errno = CANNOT_READ;
        sprintf(pico->error_text, "Cannot read hash.");
        return true;
    }
    pico->hash_valid = true;
    if (fread(&pico->key_length, 1, sizeof(keylen_t), pico->file) <
            sizeof(keylen_t)) {
        pico->errno = CANNOT_READ;
        sprintf(pico->error_text, "Cannot read key length.");
        return true;
    }
    pico->key_length = NTOH(keylen_t, pico->key_length);
    if (pico->key_length < 1) {
        pico->errno = KEY_ERROR;
        sprintf(pico->error_text, "Key has illegal length (0).");
        return true;
    }
    pico->key = MALLOC(uint8_t, pico->key_length);
    if (pico->key == NULL) {
        pico->errno = NO_MEMORY;
        sprintf(pico->error_text, "Cannot get memory to store key.");
        return true;
    }
    if (fread(pico->key, 1, pico->key_length, pico->file) <
            pico->key_length) {
        pico->errno = CANNOT_READ;
        sprintf(pico->error_text, "Cannot read key.");
        return true;
    }
    if (pico->offset < KEY_POS + pico->key_length) {
        pico->errno = BAD_OFFSET;
        sprintf(pico->error_text, "Offset is too small.");
        return true;
    }

    // Header has been read successfully.  Compute and save the
    // metadata length.
    pico->md_length = pico->offset - KEY_POS - pico->key_length;
    pico->errno = OK;
    return false;
}

PICO *
pico_open(FILE * file) {
    // Allocate immediately.
    PICO * pico = MALLOC(PICO, 1);
    if (pico == NULL) return NULL;

    // Store what we know.
    pico->file = file;

    // Read the header.  Errors are communicated in the Pico structure
    // so we do not need to check anything here.
    read_header(pico);
    return pico;
}

pico_errno
pico_finish(PICO * pico) {
    if (pico == NULL) return false;
    if (pico->file != NULL) {
        if (!pico->hash_valid) {
            pico_get_hash(pico);
            write_header(pico);
        }
        if (fflush(pico->file)) {
            pico->errno = CANNOT_WRITE;
            sprintf(pico->error_text, "Unable to flush stream.");
        } else {
            pico->file = NULL;
            pico->errno = OK;
        }
    }
    pico->key_length = 0;
    pico_free(pico->key);
    pico->key = NULL;
    pico_free(pico);
    return pico->errno;
}

//======================================================================
// Pico error handling.
//======================================================================

bool
pico_is_error(PICO * pico) {
    return pico == NULL || pico->errno != OK;
}

pico_errno
pico_get_errno(PICO * pico) {
    return pico->errno;
}

char *
pico_print_error(PICO * pico) {
    return pico->error_text;
}

void
pico_clear_error(PICO * pico) {
    pico->errno = OK;
}

//======================================================================
// Pico metadata information.
//======================================================================

size_t
pico_get_md_length(PICO * pico) {
    if (pico != NULL) return pico->md_length; else return 0;
}

uint8_t *
pico_get_metadata(PICO * pico, offset_t position, offset_t length) {
    if (length == 0) return NULL;
    // Allocate the buffer.
    uint8_t * buffer = MALLOC(uint8_t, length);
    if (buffer == NULL) {
        pico->errno = NO_MEMORY;
        sprintf(pico->error_text, "Unable to allocate buffer.");
        return NULL;
    }
    if (pico != NULL) {
        // Compute the start of the metadata to read.
        offset_t start = position + KEY_POS + pico->key_length;
        // Compute the number of bytes we can read.
        offset_t bytes = pico->offset - start;
        if (length < bytes) bytes = length;
        // Perform the read.
        if (fseek(pico->file, start, SEEK_SET) ||
                fread(buffer, 1, bytes, pico->file)) {
            pico->errno = CANNOT_READ;
            sprintf(pico->error_text, "Cannot read metadata.");
            return buffer;
        }
        pico->errno = OK;
    }
    return buffer;
}

bool
pico_set_metadata(PICO * pico, offset_t position, offset_t length,
                   uint8_t * md) {
    if (length == 0) {
        return false;
    }
    if (md == NULL) {
        pico->errno = OK;
        return true;
    }
    if (pico != NULL) {
        // Compute the start of the metadata to read.
        offset_t start = position + KEY_POS + pico->key_length;
        // Compute the number of bytes we can write.
        offset_t bytes = pico->offset - start;
        if (length < bytes) bytes = length;
        // Perform the write.
        if (fseek(pico->file, start, SEEK_SET) ||
                fwrite(md, 1, bytes, pico->file) != bytes) {
            pico->errno = CANNOT_WRITE;
            sprintf(pico->error_text, "Cannot write metadata.");

        }
        pico->errno = OK;
        return length != bytes;
    }
    return true;
}

//======================================================================
// Pico data access.
//======================================================================

size_t
pico_get(PICO * pico, size_t position, size_t length, uint8_t * buffer) {
    if (length == 0 || buffer == NULL || pico == NULL) return 0;

    // Compute the start of the data to read.
    size_t start = pico->offset + position;

    // Get the length of the data and compute the maximum number of
    // bytes we could read.
    fseek(pico->file, 0, SEEK_END);
    size_t bytes = ftell(pico->file) - start;
    if (length < bytes) bytes = length;
    if (bytes == 0) return 0;

    // Perform a read of the encoded bytes.
    size_t done = 0;
    if (fseek(pico->file, start, SEEK_SET)) {
        pico->errno = CANNOT_SEEK;
        sprintf(pico->error_text, "Cannot seek to start of data.");
        return 0;
    } else {
        if ((done = fread(buffer, 1, bytes, pico->file)) != bytes) {
            pico->errno = CANNOT_READ;
            sprintf(pico->error_text, "Cannot read data.");
            return 0;
        }
    }
    pico->errno = OK;

    // Decode the bytes.
    crypt(buffer, bytes, pico->key, pico->key_length, position);
    return bytes;
}

size_t
pico_set(PICO * pico, size_t position, size_t length, uint8_t * data) {
    static size_t next_position = 0;
    if (length == 0 || data == NULL || pico == NULL) return 0;

    // Copy and encrypt the data prior to writing.
    uint8_t * copy = MALLOC(uint8_t, length);
    if (copy == NULL) {
        pico->errno = NO_MEMORY;
        sprintf(pico->error_text, "Unable to allocate buffer.");
        return 0;
    }
    memcpy(copy, data, length);
    crypt(copy, length, pico->key, pico->key_length, position);
    // Move to the correct position in the file and write the data.  We
    // also invalidate the hash here.  Maybe.  If we write sequentially
    // then there is no need; we can just update the hash.  Right now
    // that is not implemented, but it should be relatively easy.  We
    // make the hash stuff global to the module, and then update it if
    // we have position = next_position.
    pico->hash_valid = false;
    fseek(pico->file, position + pico->offset, SEEK_SET);
    size_t actual = fwrite(copy, 1, length, pico->file);
    next_position = position + actual;
    if (actual != length) {
        pico->errno = CANNOT_WRITE;
        sprintf(pico->error_text, "Cannot write data.");
        return actual;
    }
    pico->errno = OK;
    return length;
}

//======================================================================
// Pico whole file operations.
//======================================================================

pico_errno
pico_encode_file(char *infile, char *outfile, keylen_t key_length, uint8_t *key,
                 offset_t md_length, FILE *err) {
    if (infile == NULL) {
        ERR(err, "ERROR: Input file name is NULL.\n");
        return USAGE;
    }
    if (outfile == NULL) {
        ERR(err, "ERROR: Output file name is NULL.\n");
        return USAGE;
    }

    // Allocate the buffer.
    uint8_t * buffer = MALLOC(uint8_t, CHUNK_SIZE);
    if (buffer == NULL) {
        ERR(err, "ERROR: Unable to allocate buffer.\n");
        return NO_MEMORY;
    }

    // Open the file to encode.
    FILE * fin = fopen(infile, "r");
    if (fin == NULL) {
        ERR(err, "ERROR: Unable to open input file.\n");
        return CANNOT_READ;
    }

    // Open the file to get output.
    FILE * fout = fopen(outfile, "w+");
    if (fout == NULL) {
        fclose(fin);
        ERR(err, "ERROR: Unable to open output file.\n");
        return CANNOT_WRITE;
    }
    PICO * pico = pico_new(fout, key_length, key, md_length);
    if (pico_is_error(pico)) {
        fclose(fin);
        fclose(fout);
        ERR(err, "ERROR: %s\n", pico->error_text);
        return pico->errno;
    }

    // Read and copy chunks until we fail to read.
    size_t count= 0;
    size_t position = 0;
    do {
        // Read a chunk.
        count = fread(buffer, 1, CHUNK_SIZE, fin);
        if (count == 0) break;

        // Write a chunk.
        pico_set(pico, position, count, buffer);
        position += count;
        if (pico_is_error(pico)) {
            pico_finish(pico);
            fclose(fout);
            fclose(fin);
            ERR(err, "ERROR: %s\n", pico->error_text);
            return pico->errno;
        }
    } while (count == CHUNK_SIZE);

    // Done.
    pico_finish(pico);
    fclose(fout);
    fclose(fin);
    return OK;
}

pico_errno
pico_decode_file(char *infile, char *outfile, bool testhash, FILE *err) {
    if (infile == NULL) {
        ERR(err, "ERROR: Input file name is NULL.\n");
        return USAGE;
    }
    if (outfile == NULL) {
        ERR(err, "ERROR: Output file name is NULL.\n");
        return USAGE;
    }

    // Allocate the buffer.
    uint8_t * buffer = MALLOC(uint8_t, CHUNK_SIZE);
    if (buffer == NULL) {
        ERR(err, "ERROR: Unable to allocate buffer.\n");
        return NO_MEMORY;
    }

    // Open the file to decode.
    FILE * fin = fopen(infile, "r");
    if (fin == NULL) {
        ERR(err, "ERROR: Unable to open input file.\n");
        return CANNOT_READ;
    }

    // Open the file to get output.
    FILE * fout = fopen(outfile, "w+");
    if (fout == NULL) {
        fclose(fin);
        ERR(err, "ERROR: Unable to open output file.\n");
        return CANNOT_WRITE;
    }

    // Read the header.
    PICO * pico = pico_open(fin);
    if (pico_is_error(pico)) {
        fclose(fin);
        fclose(fout);
        ERR(err, "ERROR: %s\n", pico->error_text);
        return pico->errno;
    }

    // Initialize the hash computation.
    uint8_t hash[HASH_LEN];
    MD5_CTX md5data;
    if (testhash) MD5_Init(&md5data);

    // Read and copy chunks until we fail to read.
    size_t count = 0;
    size_t position = 0;
    do {
        // Read a chunk.
        count = pico_get(pico, position, CHUNK_SIZE, buffer);
        if (count == 0) break;
        if (pico_is_error(pico)) {
            if (testhash) MD5_Final(hash, &md5data);
            pico_finish(pico);
            fclose(fin);
            fflush(fout);
            fclose(fout);
            ERR(err, "ERROR: %s\n", pico->error_text);
            return pico->errno;
        }
        if (testhash) MD5_Update(&md5data, buffer, count);

        // Write a chunk.
        fwrite(buffer, 1, count, fout);
        position += count;
    } while (count == CHUNK_SIZE);

    // Check the hash.
    if (testhash) {
        bool hash_match = true;
        MD5_Final(hash, &md5data);
        for (size_t index = 0; index < HASH_LEN; ++index) {
            if (hash[index] != pico->hash[index]) {
                // Hash mismatch.
                hash_match = false;
            }
        } // Loop over hash bytes.
    }

    // Done.
    pico_finish(pico);
    fclose(fin);
    fflush(fout);
    fclose(fout);
    return hash_match ? OK : HASH_ERROR;
}
