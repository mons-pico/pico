/**
 *        _
 *   _ __(_)__ ___
 *  | '_ \ / _/ _ \
 *  | .__/_\__\___/
 *  |_|            Pico
 *
 * @file
 * Common utilities.
 *
 * @author Stacy Prowell (prowellsj@ornl.gov)
 *
 * @section LICENSE
 * This program is Copyright (c) by UT-Battelle, LLC.
 * All rights reserved.
 */

#import <iostream>
#import <iomanip>
#import <sstream>
#import <string>
#import <ctime>
#import <stdexcept>
#import "dirent.h" // For opendir, etc.
#import "sys/stat.h" // For stat.
#import "unistd.h" // For unlink.
#import "errno.h" // For error codes.
#import "stdio.h" // For remove.

using std::string;
using std::ios;
using std::ostringstream;

namespace pico {

std::ostream&
timestamp(std::ostream& out)
{
	time_t ticks;
	time(&ticks);
	return out << ctime(&ticks);
}

int
delete_directory(const string& path, bool verbose, bool simulate)
{
	// Open the directory.
	DIR *dir = opendir(path.c_str());
	if (dir == 0L) {
		return ENOTDIR;
	}
	// Read every element of the directory and delete it.
	int retval;
	struct dirent *entry = 0L;
	while ((entry = readdir(dir)) != 0L) {
		// If we hit null, stop.
		if (entry == 0L) {
			break;
		}
		// Always ignore dot and dot-dot!
		size_t len = strlen(entry->d_name);
		if (len > 0 && entry->d_name[0] == '.') {
			if (len == 1 || (len == 2 && entry->d_name[1] == '.')) {
				continue;
			}
		}
		// Find out if we have another directory.  We have to recursively
		// delete it, if so.
		string fullname = path + "/" + entry->d_name;
		struct stat info;
		if ((retval = stat(fullname.c_str(), &info)) != 0) {
			// Stat failed on the file.  Stop and fail.
			return retval;
		}
		// See if the entry is a directory.
		if (info.st_mode & S_IFDIR) {
			// Recursively delete.
			if ((retval = delete_directory(fullname, verbose, simulate)) != 0) {
				return retval;
			}
		} else {
			// Just a file.  Try to delete it with remove.
			if (verbose)
				std::cout << "delete " << fullname << std::endl << std::flush;
			if ((!simulate) && ((retval = remove(fullname.c_str())) != 0)) {
				return retval;
			}
		}
	} // Delete all entries in the directory.

	// Done with the directory.
	closedir(dir);

	// Delete the directory.
	if (verbose)
		std::cout << "rmdir " << path << std::endl << std::flush;
	if ((!simulate) && ((retval = remove(path.c_str())) != 0)) {
		return retval;
	}

	// Success!
	return 0;
}

}
