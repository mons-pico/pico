/**
 * @file
 * Test crypt functions.
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
#include <assert.h>

/**
 * Returns an integer in the range [0, n).  Uses rand(), and so is
 * affected-by / affects the same seed.
 *
 * This code is taken from an answer on Stack Overflow.
 * http://stackoverflow.com/questions/822323/how-to-generate-a-randlong-number-in-c
 *
 * @param n         Upper exclusive bound.
 * @return          The random number chosen.
 */
long randlong(long n) {
    if (n == 0) return 0;
    if ((n - 1) == RAND_MAX) {
        return rand();
    } else {
        // Chop off all of the values that would cause skew...
        long end = RAND_MAX / n; // truncate skew
        assert (end > 0L);
        end *= n;

        // ... and ignore results from rand() that fall above that limit.
        // (Worst case the loop condition should succeed 50% of the time,
        // so we can expect to bail out of this loop pretty quickly.)
        long r;
        while ((r = rand()) >= end);

        return r % n;
    }
}

// Provide access to the otherwise private function for encrypting data.
uint8_t * pico_crypt(uint8_t *data, size_t len, uint8_t *key, keylen_t keylen,
                     size_t position);

#define LEN 32768
START_TEST;
        START_ITEM(crypt);
                // Randomize.
                srand(time(NULL));

                // Keys to try.
                uint8_t key0[] = { 0, 0, 0, 0, 0, };
                uint8_t key1[] = { 255, 255, 255, 255, 255, };
                uint8_t key2[] = { 0x55, 0x21, 0xe8, 0x6c, 0x8b, };

                // Some data lengths.  Try encrypting and decrypting data of
                // these different lengths.
                size_t lengths[] = { 1, 2, 4, 8, 17, 31, 4095, 8193, 16385 };
                int length_count = 9;
                uint8_t data[LEN];
                uint8_t copy[LEN];
                WRITELN("Starting test.");
                for (int size = 0; size < length_count; ++size) {
                    WRITE("Generating size %ld... ", lengths[size]);
                    for (size_t index = 0; index < lengths[size]; ++index) {
                        data[index] = (uint8_t) randlong(256);
                    } // Randomly generate data.
                    for (size_t index = lengths[size]; index < LEN; ++index) {
                        data[index] = 0;
                    } // Fill out the array with nuls.
                    memcpy(copy, data, LEN);
                    WRITELN("Done.");

                    // Encrypt with zero key.  No difference.
                    WRITE("Testing with all zero key... ");
                    pico_crypt(data, lengths[size], key0, 5, size);
                    if (memcmp(data, copy, LEN) != 0) {
                        FAIL("Bad encryption of data for length %ld.", lengths[size]);
                    }
                    // Decrypt with zero key.  No difference.
                    pico_crypt(data, lengths[size], key0, 5, size);
                    if (memcmp(data, copy, LEN) != 0) {
                        FAIL("Bad decryption of data for length %ld.", lengths[size]);
                    }
                    WRITELN("Done.");

                    // Encrypt and decrypt with other keys.
                    WRITE("Testing symmetry with other keys... ");
                    pico_crypt(data, lengths[size], key1, 5, size);
                    pico_crypt(data, lengths[size], key1, 5, size);
                    pico_crypt(data, lengths[size], key2, 5, size);
                    pico_crypt(data, lengths[size], key2, 5, size);
                    if (memcmp(data, copy, LEN) != 0) {
                        FAIL("Bad (en/de)cryption of data for length %ld.", lengths[size]);
                    }
                    WRITELN("Done.");
                } // Try all data lengths.
        END_ITEM
END_TEST
