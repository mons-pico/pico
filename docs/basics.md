# Basics

To use the library add the `include` folder and the built library (static or
dynamic) to your project.  You only need to `#include <pico.h>`.

## Whole-file operations

If you just want to encode or decode a file, you can use `encode_file` and
`decode_file`, respectively.  These take the file names, the key (if encoding),
and a stream to get error messages.

```c
uint8_t key[] = { 0x21, 0x55, 0x18, 0xfe };
encode_file("input.dat", "input.pico", 4, key, 0, stderr);
decode_file("input.pico", "output.dat", stderr);
```

## Working with an existing Pico-encoded file

Open a file in the usual way.  If you only intend to read, you can open it
with `"r"`, but if you want to write you must use `"w+"`, and in either case
the file must be available for random access (`fseek` must work).

```c
FILE * file = fopen("myfile.pico", "r");
```

Next you must wrap the file pointer to get a Pico pointer.

```c
PICO * pico = pico_open(file);
```

When you are done working with the file, you must first discard the Pico
pointer as follows, and then close the file.

```c
pico_finish(pico);
fclose(file);
```

## Creating a new Pico-encoded file

To create a new Pico-encoded file, open a file for writing and then use
`pico_new` to get a Pico pointer.  You must supply the encryption key to
use, and specify the amount of space to reserve for metadata (if any).

```c
uint8_t key[] = { 0x21, 0x55, 0x18, 0xfe };
FILE * file = fopen("newfile.pico", "w+");
PICO * pico = pico_new(file, 4, key, 0);

// Work with file...

pico_finish(pico);
fclose(file);
```

## Reading and writing

To read and write data, use `pico_get` and `pico_put`, respectively.  These
two operate on a buffer of `uint8_t` and automatically decode / encode the
data before returning it.

```c
// Read the first 16KB of data from a Pico-encoded file.  Position 0 in the
// file corresponds to the first byte of the encoded data.  The return value
// is the number of bytes actually read (it may be less if the file contains
// less).
size_t chunk = 16384;
uint8_t data[chunk];
size_t bytes_read = pico_get(pico, 0, chunk, data);
```

## Reading and writing metadata

You can store metadata in a Pico-encoded file in any format you want.  Space
must be reserved when the Pico-file is created, and if you need more space you
must re-create the file.  To reserve space pass the number of bytes as an
argument to `pico_new`.  To get the reserved metadata length in bytes, use
`pico_get_md_length`.

Read from the metadata with `pico_get_metadata` and write to the metadata with
`pico_set_metadata`.

It is suggested that metadata be stored as a string.

## Error checking and reporting

The Pico functions set and report errors via the Pico data structure.  To check
whether a function generated an error, you can use `pico_is_error`.  You can
print the error to `stderr` via `pico_print_error`, or check the error code
yourself via `pico_get_errno`.

```c
PICO * pico = pico_open(file);
if (pico == NULL) {
    fprintf(stderr, "Unable to open file.\n");
} else if (pico_is_error(pico)) {
    pico_print_error(pico);
} else {
    // Everything is okay...
}
```

## Accessing header information

Header information can be accessed, but not really "set."  This is because the
header information is either fixed (like the version numbers) or computed (like
the offset and hash).  The exceptions are the key and metadata length (which
determines the offset), but those can only be set when a file is created.

  * The "magic number" is the first two bytes of the file.  This is used to
    indicate that the file is a Pico-encoded file.  This is always 0x91 0xC0,
    and you can access it with `pico_magic`.
    
  * The version number of the Pico-encoding supported by the library can be
    accessed via `pico_major` and `pico_minor`.  Pico uses Semantic Versioning,
    so the major version number determines compatibility.
  
  * The version number of a Pico-encoded file can be accessed via
    `pico_get_major` and `pico_get_minor`.
    
  * The offset (the zero-based index of the first encoded byte of data in the
    file) can be obtained via `pico_get_offset`.
    
  * The MD5 hash of the stored data, computed over the decoded bytes, can be
    accessed via `pico_get_hash`.  This is the MD5 hash of the original
    (unencrypted) file.  Do not deallocate the returned hash array.
    
  * You can get the key that is used to encrypt and decrypt the data via the
    two functions `pico_get_key` and `pico_get_key_length`.  Do not deallocate
    the returned key array!
    
If you want to see what is in the header of a Pico-encoded file you can write
the header information using `pico_dump_header`.  This can write the header
as XML, JSON, YAML, or as a Python `dict`.

```c
// Write the header of a Pico-encoded file specified by the command line
// argument as a Python dictionary.
#include <pico.h>
int main(int argc, char * argv[]) {
    if (argc < 2) return 0;
    FILE * file = fopen(argv[1], "r");
    if (file == NULL) {
        fprintf(stderr, "Unable to open file.");
        return 1;
    }
    PICO * pico = pico_open(file);
    if (pico == NULL) {
        fprintf(stderr, "Unable to decode file.");
        return 2;
    }
    pico_dump_header(pico, PYTHON_DICT, stdout);
    return 0;
}
```
