# Building

The current version of the library can be found on the
[Mons Pico site][mons-pico].  Go ahead check out the library with:

```bash
git clone https://github.com/mons-pico/pico.git
```

The CPico library is built using [CMake][cmake].  You'll need to have CMake
installed to build the library, and you will also (obviously) need a C
compiler.

## Building Out of Source

It is recommended that you build "out of source."  Do the following from the
top-level folder.

```bash
mkdir build
cd build
cmake ..
make
```

At the end of this you should have the library and an executable named `pico`.
All the build products are now in the `build` folder, which you can easily
delete to clean up.

## API Documentation

If CMake finds [Doxygen][doxygen], then it will also build documentation in the
`docs/api` folder.  Open `docs/api/html/index.html` to see that.

The API documentation has not been integrated with MkDocs (see next section).

## Other Documentation

The documentation you are reading now is build and can be served out using
[MkDocs][mkdocs].  If you have MkDocs installed on you computer you can do
the following from the root folder to build and serve out the documentation.

```bash
mkdocs serve
```

[mons-pico]: http://mons-pico.github.io
[cmake]: https://cmake.org
[doxygen]: http://www.stack.nl/~dimitri/doxygen/
[mkdocs]: http://www.mkdocs.org
