#include <getopt.h>
#include <string.h>
#include <pico.h>

void
print_usage(char * name) {
    fprintf(stdout, "usage: %s [flags] [input files...]\n", name);
    fprintf(stdout, "Flags:\n");
    fprintf(stdout, "  -d / --decode ............ Decode files.\n");
    fprintf(stdout, "  -e / --encode ............ Encode files (default).\n");
    fprintf(stdout, "  --extension=[ext] ........ Set output file extension.\n");
    fprintf(stdout, "  -h / --help .............. Print this help information.\n");
    fprintf(stdout, "  -H / --header ............ Dump header information.\n");
    fprintf(stdout, "  -s / --suffix=[suffix] ... Suffix to add to output files.\n");
    fprintf(stdout, "\n");
    //               0123456789012345678901234567890123456789012345678901234567890123456789
    fprintf(stdout, "Input files are read and (de/en)coded.  If encoding, a .pico extension\n");
    fprintf(stdout, "is added to the file.  If decoding, a .raw extension is added to the\n");
    fprintf(stdout, "the file.  The extension used can be overridden by --extension.  Any\n");
    fprintf(stdout, "provided suffix is added to the file's base name.\n");
    fprintf(stdout, "\n");
    fprintf(stdout, "Pico encoding version: %d.%d.\n",
        VERSION_MAJOR, VERSION_MINOR);
}

int
main(int argc, char * argv[]) {
    // Information we get from the command line.
    char * suffix = "";
    bool encode = true;
    bool header = false;
    bool quiet = false;
    bool debug = false;
    char * myname = argv[0];
    char * extension = NULL;
    int argument = 0;

    // Process the arguments.
    static const char * optstring = "dehHs:q";
    static const struct option longopts[] = {
            { "debug", no_argument, NULL, 0 },
            { "decode", no_argument, NULL, 'd' },
            { "encode", no_argument, NULL, 'e' },
            { "extension", required_argument, NULL, 0 },
            { "help", no_argument, NULL, 'h' },
            { "header", no_argument, NULL, 'H' },
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
                    debug = true;
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

    if (debug) {
        fprintf(stdout, "Encoding: %d\n", encode);
        fprintf(stdout, "Suffix: %s\n", suffix);
        fprintf(stdout, "Header: %d\n", header);
        fprintf(stdout, "Extension: %s\n", extension);
        fprintf(stdout, "Arguments:\n");
        for (int index = optind; index < argc; ++index) {
            fprintf(stdout, "  -> %s\n", argv[index]);
        } // Print arguments.
    }

    // Process all files.
    offset_t md_length = 0;
    keylen_t keylen = 4;
    uint8_t key[4] = { 0x43, 0x2E, 0xE5, 0x04 };
    const size_t addlen = strlen(extension) + strlen(suffix);
    for (int index = optind; index < argc; ++index) {
        if (strlen(argv[index]) == 0) continue;

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
            fprintf(stderr, "Refusing to overwrite input file %s.\n",
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
        if (encode) encode_file(argv[index], outname, keylen, key,
                                md_length, stderr);
        else decode_file(argv[index], outname, header, stderr);
    } // Process all files.
    return 0;
}