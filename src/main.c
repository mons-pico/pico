/**
 * @file
 * The Pico command line executable.
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

#include <getopt.h>
#include <string.h>
#include <pico.h>
#include <ctype.h>
#include <stdlib.h>

/**
 * Print usage information for the command line executable.
 *
 * @param name    The name of the executable, from the command line arguments.
 */
static void
print_usage(char * name) {
    fprintf(stdout, "usage: %s [flags] [input files...]\n", name);
    fprintf(stdout, "Encode a file as Pico, decode a Pico-encoded file, or dump the header\n");
    fprintf(stdout, "from a Pico-encoded file.\n");
    fprintf(stdout, "\n");
    fprintf(stdout, "Flags:\n");
    fprintf(stdout, "  --debug .................. Enable debugging.\n");
    fprintf(stdout, "  -d / --decode ............ Decode files.\n");
    fprintf(stdout, "  -e / --encode ............ Encode files (default).\n");
    fprintf(stdout, "  --extension=[ext] ........ Set output file extension.\n");
    fprintf(stdout, "  -h / --help .............. Print this help information.\n");
    fprintf(stdout, "  -H / --header=[kind] ..... Dump header information.\n");
    fprintf(stdout, "  -k / --key=[key] ......... Use the given key for encryption.\n");
    fprintf(stdout, "  -s / --suffix=[suffix] ... Suffix to add to output files.\n");
    fprintf(stdout, "\n");
    //               0123456789012345678901234567890123456789012345678901234567890123456789
    fprintf(stdout, "Input files are encoded by default.  If encoding, a .pico extension is added to\n");
    fprintf(stdout, "the file.  If decoding, then the input must be Pico-encoded files, and a .raw\n");
    fprintf(stdout, "extension is added by default.  If dumping the header, the input files must be\n");
    fprintf(stdout, "Pico-encoded files, and the header is dumped as JSON to standard output.\n");
    fprintf(stdout, "\n");
    fprintf(stdout, "The extension used can be overridden by --extension, which should include the\n");
    fprintf(stdout, "dot.  Any provided suffix (by default there is none) is added to the file's\n");
    fprintf(stdout, "base name.\n");
    fprintf(stdout, "\n");
    fprintf(stdout, "The header kinds can be json, yaml, python, or xml.");
    fprintf(stdout, "\n");
    fprintf(stdout, "Keys must be specified as a list of hexadecimal digits (no spaces).\n");
    fprintf(stdout, "\n");
    fprintf(stdout, "Pico encoding version: %d.%d.\n", VERSION_MAJOR, VERSION_MINOR);
    fprintf(stdout, "Using CPico library built: %s\n", pico_build());
}

/**
 * Entry point when started from the command line.
 *
 * @param argc    Number of command line arguments.
 * @param argv    Command line arguments.
 * @return        Exit value.
 */
int
main(int argc, char * argv[]) {
    // Information we get from the command line.
    char * suffix = "";         ///< The suffix, if provided.
    char * tkey = NULL;         ///< Text key.
    uint8_t * bkey = NULL;      ///< Decoded "binary" key.
    keylen_t keylen = 0;        ///< Key length in bytes.
    bool encode = true;         ///< True to encode.
    bool header = false;        ///< True to dump header.
    bool quiet = false;         ///< True to suppress messages.
    char * myname = argv[0];    ///< The name of this executable.
    char * extension = NULL;    ///< The extension, if provided.
    int argument = 0;           ///< An argument.
    header_format_t kind = PYTHON_DICT;
    uint8_t default_key[8] = {
            (uint8_t)0x21, (uint8_t)0x18, (uint8_t)0xF5, (uint8_t)0xE5,
            (uint8_t)0x59, (uint8_t)0x64, (uint8_t)0x79, (uint8_t)0xAA,
    };
    keylen_t default_keylen = sizeof(default_key) / sizeof(uint8_t);

    // Process the arguments.
    static const char * optstring = "dehH:s:q";
    static const struct option longopts[] = {
            { "debug", no_argument, NULL, 0 },
            { "decode", no_argument, NULL, 'd' },
            { "encode", no_argument, NULL, 'e' },
            { "extension", required_argument, NULL, 0 },
            { "help", no_argument, NULL, 'h' },
            { "header", required_argument, NULL, 'H' },
            { "key", required_argument, NULL, 'k' },
            { "suffix", required_argument, NULL, 's' },
            { "quiet", no_argument, NULL, 'q' },
            { NULL, 0, NULL, 0 }
    };
    int opt = getopt_long(argc, argv, optstring, longopts, &argument);
    opterr = 0;
    while (opt != -1) {
        switch (opt) {
            case 'd':
                encode = false;
                break;

            case 'e':
                encode = true;
                break;

            case 'h':
                print_usage(myname);
                return 0;

            case 'H':
                header = true;
                if (strcmp(optarg, "json") == 0) {
                    kind = JSON;
                } else if (strcmp(optarg, "yaml") == 0) {
                    kind = YAML;
                } else if (strcmp(optarg, "python") == 0) {
                    kind = PYTHON_DICT;
                } else if (strcmp(optarg, "xml") == 0) {
                    kind = XML;
                } else {
                    fprintf(stderr, "ERROR: Unknown header format %s.\n", optarg);
                    fprintf(stderr, "Legal values are:\n");
                    fprintf(stderr, "  python json yaml xml\n");
                    return 1;
                }
                break;

            case 'k':
                for (tkey = optarg; *tkey = (char) tolower(*tkey), *tkey; ++tkey);
                tkey = optarg;
                {
                    size_t len = strlen(tkey);
                    if (len % 2 != 0) {
                        fprintf(stderr, "ERROR: Key must be an even number of hex digits.\n");
                        return 1;
                    }
                    if (len == 0) {
                        fprintf(stderr, "ERROR: Key cannot be empty.\n");
                        return 1;
                    }
                    if (len > 64) {
                        fprintf(stderr, "ERROR: Key length cannot be greater than 32 bytes (64 digits).\n");
                        return 1;
                    }
                    bkey = (uint8_t *)calloc(sizeof(uint8_t), len / 2);
                    for (size_t index = 0; index < len / 2; ++index) {
                        uint8_t d1 = (uint8_t)tkey[index*2] - (uint8_t)'0';
                        uint8_t d2 = (uint8_t)tkey[index*2+1] - (uint8_t)'0';
                        if (d1 > 9) d1 += ((uint8_t)'0' - (uint8_t)'a' + 10);
                        if (d2 > 9) d2 += ((uint8_t)'0' - (uint8_t)'a' + 10);
                        if (d1 < 0 || d1 > 15 || d2 < 0 || d2 > 16) {
                            fprintf(stderr, "ERROR: Non-hex digit in key at pair %1c%1c.\n",
                                    tkey[index*2], tkey[index*2+1]);
                            return 1;
                        }
                        bkey[index] = d1*(uint8_t)16+d2;
                        ++keylen;
                    } // Convert all digits.
                }
                break;

            case 's':
                suffix = optarg;
                break;

            case 'q':
                quiet = true;
                break;

            case 0:
                if (strcmp("extension", longopts[argument].name) == 0) {
                    extension = optarg;
                } else if (strcmp("debug", longopts[argument].name) == 0) {
                    pico_debug = 1;
                    fprintf(stdout, "Debugging enabled.\n");
                }
                break;

            default:
                fprintf(stderr, "Try -h for usage information.\n");
                return 1;
        }

        opt = getopt_long(argc, argv, optstring, longopts, &argument);
    } // Loop over command line arguments.

    // See if we are using default extension.
    if (extension == NULL) {
        extension = (encode ? ".pico" : ".raw");
    }

    // If there was no key, use a default one now.
    if (bkey == NULL) {
        bkey = default_key;
        keylen = default_keylen;
    }

    if (pico_debug) {
        DEBUG("Encoding: %d", encode);
        DEBUG("Suffix: %s", suffix);
        DEBUG("Header: %d", header);
        DEBUG("Extension: %s", extension);
        DEBUG("Key: ");
        for (size_t index = 0; index < keylen; ++index) {
            DEBUG("    %02X", *(bkey + index));
        } // Write all bytes.
        DEBUG("Arguments:");
        for (int index = optind; index < argc; ++index) {
            DEBUG("  -> %s", argv[index]);
        } // Print arguments.
    }

    // Process all files.
    offset_t md_length = 0;
    const size_t addlen = strlen(extension) + strlen(suffix);
    for (int index = optind; index < argc; ++index) {
        if (strlen(argv[index]) == 0) continue;

        // If the user wants only the header, then just do that.  The remainder
        // of this loop body assumes that you are either encoding or decoding.
        if (header) {
            // Open the input file, which should be a Pico-encoded file.
            FILE * fin = fopen(argv[index], "r");
            if (fin == NULL) {
                fprintf(stderr, "ERROR: Cannot locate input file: %s.\n",
                        argv[index]);
                continue;
            }
            PICO * pico = pico_open(fin, NULL);
            if (pico == NULL) {
                fprintf(stderr, "ERROR: File is not a Pico file: %s.\n",
                        argv[index]);
                continue;
            }
            if (! pico_is_error(pico)) {
                pico_dump_header(pico, kind, stdout);
            }
            if (pico_is_error(pico)) {
                fprintf(stderr, "ERROR: %s\n", pico->error_text);
            }
            pico_finish(pico);
            fclose(fin);
            continue;
        }

        // Compute the appropriate output filename.
        const size_t maxlen = addlen + strlen(argv[index]) + 1;
        char outname[maxlen];
        strcpy(outname, argv[index]);
        char * dot = strrchr(outname, '.');
        if (dot != NULL) {
            // Remove the extension.
            strcpy(dot, suffix);
        } else {
            strcat(outname, suffix);
        }
        strcat(outname, extension);

        // Watch for clobbering.
        if (strcmp(outname, argv[index]) == 0) {
            fprintf(stderr, "WARNING: Refusing to overwrite input file %s.\n",
                    argv[index]);
            continue;
        }

        // Tell the user what we are going to do.
        if (! quiet) {
            fprintf(stdout, "  %s: %s --> %s\n",
                    (encode ? "Encoding" : "Decoding"),
                    argv[index], outname);
        }

        // Process the file.
        if (encode) {
            pico_errno errno =
                    pico_encode_file(argv[index], outname, keylen, bkey,
                                     md_length, stderr);
            if (errno != OK) {
                fprintf(stderr, "ERROR: Error detected on file encode.\n");
            }
        } else {
            pico_errno errno =
                    pico_decode_file(argv[index], outname, true, stderr);
            if (errno != OK) {
                if (errno == HASH_ERROR) {
                    fprintf(stderr, "WARNING: Decoded hash does not match.\n");
                } else {
                    fprintf(stderr, "ERROR: Error detected on file decode.\n");
                }
            }
        }
    } // Process all files.
    return 0;
}
