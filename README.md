# C Pico library

|Branch      |Status   |
|------------|---------|
|master      | [![Build Status](https://travis-ci.org/sprowell/pico.svg?branch=master)](https://travis-ci.org/sprowell/pico) |
|dev         | [![Build Status](https://travis-ci.org/sprowell/pico.svg?branch=dev)](https://travis-ci.org/sprowell/pico) |

**Branch Policy:**

 - The "master" branch is intended to be stable at all times.
 - The "dev" branch is the one where all contributions must be merged before
   being promoted to master.
    + If you plan to propose a patch, please commit into the "dev" branch or a
      separate feature branch.  Direct commit to "master" will not be accepted.

The Pico encoding is intended for packaging malicious software to prevent it
from running, and also to protect it from detection and damage due to
anti-malware software.  The format is simple and (unlike compression) provides
quick direct access to the unencrypted bytes.

Read about the format [here][mons-pico].

This library provides a C API for Pico encoding and decoding files, and for
working with those files.

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
