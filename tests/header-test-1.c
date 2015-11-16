/**
 * @file
 * Test the Pico file header.
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

#include "test-frame.h"
#include <pico.h>
#include <sys/errno.h>

typedef uint8_t byte;
START_TEST;
START_ITEM(header1)
{

        // Define the content of a test file.
        uint8_t key[] = {
                (uint8_t) 0x00, (uint8_t) 0x11,
                (uint8_t) 0x77, (uint8_t) 0x55,
                (uint8_t) 0xff, (uint8_t) 0xa8,
                (uint8_t) 0x23, (uint8_t) 0x26,
                (uint8_t) 0xa4, (uint8_t) 0x3e,
                (uint8_t) 0x2a, (uint8_t) 0x7a,
                (uint8_t) 0x43,
        };
        keylen_t keylen = sizeof(key) / sizeof(uint8_t);

        uint8_t testdata[] = {
                (uint8_t) 'T', (uint8_t) 'h',
                (uint8_t) 'i', (uint8_t) 's',
                (uint8_t) ' ', (uint8_t) 'i',
                (uint8_t) 's', (uint8_t) ' ',
                (uint8_t) 'a', (uint8_t) ' ',
                (uint8_t) 'T', (uint8_t) 'E',
                (uint8_t) 'S', (uint8_t) 'T',
                (uint8_t) '.', (uint8_t) 0x0a,
                (uint8_t) 0x00, (uint8_t) 0xff,
                (uint8_t) 0x55, (uint8_t) 0xaa,
                (uint8_t) 'A', (uint8_t) ' ',
                (uint8_t) 't', (uint8_t) 'e',
                (uint8_t) 's', (uint8_t) 't',
                (uint8_t) ' ', (uint8_t) 't',
                (uint8_t) 'h', (uint8_t) 'i',
                (uint8_t) 's', (uint8_t) ' ',
                (uint8_t) 'i', (uint8_t) 's',
                (uint8_t) '.', (uint8_t) 0x37,
        };

        uint8_t encoded[] = {
                0x91, 0xc0, 0x00, 0x01, 0x00, 0x00,
                0x00, 0x00,
                0x00, 0x29, 0x9f, 0x20, 0x7f, 0x81,
                0x09, 0xbe,
                0xe5, 0x4d, 0x7f, 0xc9, 0xd4, 0x04,
                0xd6, 0xdf,
                0xca, 0x20, 0x00, 0x0d, 0x00, 0x11,
                0x77, 0x55,
                0xff, 0xa8, 0x23, 0x26, 0xa4, 0x3e,
                0x2a, 0x7a,
                0x43, 0x54, 0x79, 0x1e, 0x26, 0xdf,
                0xc1, 0x50,
                0x06, 0xc5, 0x1e, 0x7e, 0x3f, 0x10,
                0x54, 0x3f,
                0x7d, 0x55, 0x00, 0xfd, 0x89, 0x67,
                0x84, 0x4a,
                0x4f, 0x09, 0x37, 0x20, 0x65, 0x1f,
                0x3c, 0x8c,
                0x88, 0x4a, 0x55, 0x8a, 0x09,
        };

        /* The above data, when correctly in a Pico wrapper file, looks like this.
         *
         * 0000000 91 c0 00 01 00 00 00 00 00 29 9f 20 7f 81 09 be
         * 0000010 e5 4d 7f c9 d4 04 d6 df ca 20 00 0d 00 11 77 55
         * 0000020 ff a8 23 26 a4 3e 2a 7a 43 54 79 1e 26 df c1 50
         * 0000030 06 c5 1e 7e 3f 10 54 3f 7d 55 00 fd 89 67 84 4a
         * 0000040 4f 09 37 20 65 1f 3c 8c 88 4a 55 8a 09
         *
         * 0000000 91 c0 --> magic string
         * 0000002 00 00 --> major version number (1)
         * 0000004 00 00 --> minor version number (0)
         * 0000006 00 00 00 29 --> offset to data
         * 000000a 9f 20 7f 81 09 be ef 4d 7f c9 d4 04 d6 df ca 20 --> hash (md5)
         * 000001a 00 0d --> key length (13)
         * 000001c 00 11 77 55 ff a8 23 26 a4 3e 2a 7a 43 --> key
         * 0000029 54 79 1e 26 .. 09 --> encrypted data
         */

        WRITELN("Size of key: %ld.",
                sizeof key / sizeof *key);
        WRITELN("Size of testdata: %ld.",
                sizeof testdata / sizeof *testdata);

        // Make a temporary file with the unencoded data.
        char origname[64];
        strcpy(origname, "pico1.XXXXXXXX");
        int fd;
        FILE * origfile;
        if ((fd = mkstemp(origname)) == -1 ||
            (origfile = fdopen(fd, "w+")) == NULL) {
                if (fd != -1) {
                        unlink(origname);
                        close(fd);
                }
                FAIL_TEST("Unable to create temporary file %s: %s\n",
                     origname, strerror(errno));
        }
        fwrite(testdata, 1,
               sizeof testdata / sizeof *testdata,
               origfile);
        fflush(origfile);
        fclose(origfile);

        // Encode the file.
        char piconame[64];
        strcpy(piconame, "pico2.XXXXXXXX");
        FILE * picofile;
        if ((fd = mkstemp(piconame)) == -1 ||
            (picofile = fdopen(fd, "w+")) == NULL) {
                if (fd != -1) {
                        unlink(piconame);
                        close(fd);
                }
                unlink(origname);
                FAIL_TEST("Unable to create temporary file %s: %s\n",
                     piconame, strerror(errno));
        }
        fwrite(encoded, 1,
               sizeof encoded / sizeof *encoded,
               picofile);
        fflush(picofile);
        fclose(picofile);

        // Now generate the Pico-encoded version of the original file.
        char genname[64];
        strcpy(genname, "pico3.XXXXXXXX");
        if ((fd = mkstemp(genname)) == -1) {
                unlink(origname);
                unlink(piconame);
                FAIL_TEST("Unable to create temporary file %s: %s\n",
                     genname, strerror(errno));
        }
        close(fd);

        // Encode the original file and compare.
        pico_encode_file(origname, genname, keylen, key, 0, stderr);
        FILE * genfile;
        picofile = fopen(piconame, "r");
        if (picofile == NULL) {
                unlink(origname);
                unlink(piconame);
                unlink(genname);
                FAIL_TEST("Unable to open temporary file %s: %s\n",
                          piconame, strerror(errno));
        }
        genfile = fopen(genname, "r");
        if (genfile == NULL) {
                fclose(genfile);
                unlink(origname);
                unlink(piconame);
                unlink(genname);
                FAIL_TEST("Unable to open temporary file %s: %s\n",
                          genname, strerror(errno));
        }
        size_t index = 0;
        do {
                int chp = fgetc(picofile);
                int chg = fgetc(genfile);
                if (chp != chg) {
                        FAIL_TEST("Encoded file mismatch at position %ld.", index);
                }
                if (chp < 0) break;
                ++index;
        } while (1);
        fclose(genfile);
        fclose(picofile);

        // Get rid of the temporary files.
        unlink(origname);
        unlink(piconame);
        unlink(genname);
}
END_ITEM;
END_TEST;
