#ifndef KEYS_H_
#define KEYS_H_

/**
 *        _
 *   _ __(_)__ ___
 *  | '_ \ / _/ _ \
 *  | .__/_\__\___/
 *  |_|            Pico
 *
 * @file
 * Provide definitions for manipulating and applying keys for Pico encoding.
 *
 * @author ysp
 * @created Jan 1, 2013
 *
 * @section LICENSE
 * This program is Copyright (c) by UT-Battelle, LLC.
 * All rights reserved.
 */

#include <stdint.h>


namespace pico {

class KeyUtils
{
public:
	/// The default size of a randomly-generated key. */
	static const uint16_t KEYSIZE = 31;

	/// The source used for random keys.  It shouldn't have to be awesome.

};

#endif /* KEYS_H_ */
