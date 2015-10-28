# CPico Documentation

This is the documentation for the C library implementing the
[Pico encoding][mons-pico].  This is a very simple encoding designed to allow
the contents to be easily encoded and decoded in a byte-addressible manner.
The primary use case is packaging malware to avoid detection; it is completely
useless as a stealth method, however, since the format is published here and
the key is stored in the file itself.

Using this approach you can have malware on your machine and perform analysis
on it, while also having endpoint detection to protect you from - well, malware.

## License

<pre>
      _
 _ __(_)__ ___
| '_ \ / _/ _ \
| .__/_\__\___/
|_|            Pico
</pre>

CPico is Copyright (c) 2015 by Stacy Prowell, all rights reserved.  CPico is
distributed under the [2-clause "simplified" BSD license](license.md).

## Acknowledgments

  * CPico uses [Semantic Versioning][semver].
  * CPico's documentation is written in [Markdown][markdown], and built using
    [MkDocs][mkdocs].
  * The code for CPico was edited in [Atom][atom] and [CLion][clion].
  * The project is built using [CMake][cmake].
  * CPico relies on the [Pico encoding][mons-pico], which is Copyright (c) by
    UT-Battelle, LLC, but unrestricted.
  * CPico includes [Alexander Peslyak MD5 implementation][md5] placed in the
    public domain, though this can be overridden to use the OpenSSL libraries.
    See `src/md5.h` for how one might do this.


[semver]: http://semver.org
[pony]:   http://www.ponylang.org
[mkdocs]: http://www.mkdocs.org
[atom]:   https://atom.io
[clion]:  https://www.jetbrains.com/clion/
[cmake]:  https://cmake.org
[mons-pico]: https://github.com/mons-pico/wiki/wiki/Pico-Encoding
[markdown]: https://daringfireball.net/projects/markdown/
[api]:    api/index.html
[md5]:    http://openwall.info/wiki/people/solar/software/public-domain-source-code/md5
