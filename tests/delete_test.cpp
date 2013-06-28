/**
 *        _
 *   _ __(_)__ ___
 *  | '_ \ / _/ _ \
 *  | .__/_\__\___/
 *  |_|            Pico
 *
 * @file
 * delete_test.cpp
 *
 * @author ysp
 * @created Jan 6, 2013
 *
 * @section LICENSE
 * This program is Copyright (c) by UT-Battelle, LLC.
 * All rights reserved.
 */

#include <sstream>
#include "test_frame.h"

using std::string;
using std::ostringstream;

START_TEST

START_ITEM(populate)
	// Create a deep folder.
	HANG("Creating a folder hierarchy");
	string folder("folder_tmp");
	string file("file_tmp");
	string md("mkdir -p ");
	string touch("touch ");
	ostringstream path;
	for (int depth = 0; depth < 20; ++depth) {
		// Make a directory.
		path << folder << depth << "/";
		if ((system((md + path.str()).c_str())) != 0) {
			FAIL_TEST("creating " << path.str());
		}
		// Populate the directory with some files.
		for (int count = 0; count < 20; ++count) {
			// Make a file.
			ostringstream fname;
			fname << touch << path.str() << file << count;
			if ((system(fname.str().c_str())) != 0) {
					FAIL_TEST("creating " << path.str());
			}
		} // Make files.
	} // Make folders.
END_ITEM(populate)

START_ITEM(simulate)
	HANG("Running simulated recursive delete");
	if (pico::delete_directory("folder_tmp0", true, true)) {
		FAIL_TEST("");
	}
END_ITEM(simulate)

START_ITEM(rmdir)
	HANG("Running verbose recursive delete");
	if (pico::delete_directory("folder_tmp0", true, false)) {
		FAIL_TEST("");
	}
END_ITEM(rmdir)

END_TEST
