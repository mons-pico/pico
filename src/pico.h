#ifndef PICO_H_
#define PICO_H_

/**
 *        _
 *   _ __(_)__ ___
 *  | '_ \ / _/ _ \
 *  | .__/_\__\___/
 *  |_|            Pico
 *
 * @file
 * Shared definitions for the Pico library.
 *
 * @author Stacy Prowell (prowellsj@ornl.gov)
 *
 * @section LICENSE
 * This program is Copyright (c) by UT-Battelle, LLC.
 * All rights reserved.
 */


/**
 * Construct a string using the << operator, and then perform the action
 * act_m, passing the string in parentheses.  This is a funny little macro
 * that can be used in several ways.
 *
 * The key idea is that << is commonly overloaded to do the Right Thing
 * with values, whereas the std::string + operator is seldom correctly
 * overloaded.  Boo!  Plus << has manipulators.
 *
 * This macro is why @<sstream@> is included.
 *
 * @b Example:
 * Throwing an exception.
 * @code
 * TRISTR(throw std::invalid_argument,
 *     "The value " << value << " is out of range.");
 * @endcode
 *
 * @b Example:
 * Assigning a string value.
 * @code
 * TRISTR(std::string msg=,
 *     "The value " << value << " is out of range.");
 * @endcode
 *
 * @b Example:
 * Passing a string argument to a function.
 * @code
 * TRISTR(foo, "The value " << value << " is out of range.");
 * @endcode
 */
#define TRISTR(act_m, exp_m) \
	{ \
		std::ostringstream msg; \
		msg << exp_m; \
		act_m(msg.str()); \
	}

/// Check an argument for null.
#define NOTNULL(x_m) \
	if (0L == x_m) { \
		throw invalid_argument("The " #x_m " is null."); \
	}

/// Path separator character as C string.  Where is this defined?
#define PATHSEP "/"

namespace pico {


/**
 * A stream manipulator to insert the current date and time in a stream.
 * Use this just like you would use std::endl.
 * @param out	The output stream.
 * @return	The output stream.
 */
std::ostream& timestamp(std::ostream& out);

/**
 * Delete a directory after first deleting all its contents.
 * @param path		The directory to delete.
 * @param verbose	If true, print each file and directory as it is removed.
 * @param simulate	If true, do not delete anything.
 * @return	Zero on success and an error code on failure.  The error codes
 * 			returned are ENOTDIR if the path is not a directory, or one of
 * 			the error codes returned by the implementation of stat and
 * 			unlink.
 */
int delete_directory(const std::string& path, bool verbose = false,
		bool simulate = false);


}

#endif /* PICO_H_ */
