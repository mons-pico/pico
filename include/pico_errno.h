#ifndef PICO_ERRNO_H
#define PICO_ERRNO_H

/**
 * @file
 * Definition of error constants used in CPico.
 */

/**
 * Enumerate the error codes used by the library.
 */
typedef enum {
    OK = 0,             //< No error detected.
    CANNOT_SEEK,        //< Unable to seek to a file location.
    CANNOT_READ,        //< The provided file could not be read.
    CANNOT_WRITE,       //< The file was not opened for writing.
    NOT_PICO,           //< The provided file does not seem to be a Pico file.
    BAD_VERSION,        //< This version of the library cannot read the file.
    NO_MEMORY,          //< The system could not provide requested memory.
    KEY_ERROR,          //< The key was NULL or length zero.
    BAD_OFFSET,         //< The offset in the header is incorrect.
    USAGE,              //< Incorrect arguments to the whole-file API.
//    HRUNG,              //< A Hrung collapsed.
} pico_errno;

#endif //PICO_ERRNO_H