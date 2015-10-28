# Library

The library uses the types defined in `stdint.h`, instead of the `int`, `long`,
etc., types.  These types are more portable.

Additionally, the header components have their own types that are used
throughout the library to avoid specifying the sizes in multiple places.  See
`pico_defs.h` for how this is done.

The library prefers `uint8_t` for bytes, instead of `char` or `unsigned char`.
This is because `char` is not necessarily a byte on every platform.  The usual
`char *` is still used for C strings.

Note that Pico specifies the byte order in the header.  Specifically, Pico uses
the "network byte order," or "big endian."  This is not necessarily the same as
the platform byte order (it probably is not the same).  The library
automatically performs conversions as necessary.

All public functions start with `pico_`, since C does not (always) provide
namespaces.

## Debugging

The library provides for debugging messages.  Set the global `pico_debug`
variable to one and then use the `DEBUG` macro.  This macro generates a
debug message if `pico_debug` is enabled, and automatically outputs a
newline.  Messages are written to `stderr`.

```c
DEBUG("Debug is set to: %d.", pico_debug);
```

This generates a message of the following form containing the source file
name and the line number where the message is generated.

```text
DEBUG (myfile.c:14): Debug is set to: 1.
```
