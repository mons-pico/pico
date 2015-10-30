# C Pico library

[![Build Status](https://travis-ci.org/sprowell/pico.svg?branch=master)](https://travis-ci.org/sprowell/pico)

The Pico encoding is intended for packaging malicious software to prevent it
from running, and also to protect it from detection and damage due to
anti-malware software.  The format is simple and (unlike compression) provides
quick direct access to the unencrypted bytes.

Read about the format [here][mons-pico].

This library provides a C API for Pico encoding and decoding files, and for
working with those files.  At present everything is implemented, except for
hashing.

More information can be found [in the docs folder](docs/).

<pre>
      _
 _ __(_)__ ___
| '_ \ / _/ _ \
| .__/_\__\___/
|_|            Pico
</pre>

CPico is Copyright (c) 2015 by Stacy Prowell, all rights reserved.  CPico is
distributed under the [2-clause "simplified" BSD license](license.md).

[mons-pico]: https://github.com/mons-pico/wiki/wiki/Pico-Encoding
