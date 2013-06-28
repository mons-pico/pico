#!/bin/bash

## Clean up the entire build area.  Really this should be a
## build target.  Someday.

RM="/bin/rm -rf"

## Usual clean.
if [ -e Makefile ] ; then make clean ; fi

## Cmake.
$RM CMakeCache.txt CMakeFiles cmake_install.cmake Makefile

## Odds and ends.
$RM pico-* lib

## Documentation.
$RM doc/api doc/wiki-backups/*

## Tests
$RM index_tmp test/index_tmp test/test.log test/test.log.*

## Bundle!
tarball='pico'`date +'%Y%m%d'`'.tgz'
(cd .. ; tar --exclude .svn -cvzf $tarball tri) >/dev/null 2>&1

## Done!
cat <<END
Build area scrubbed.  To rebuild you will need to first
run cmake and then make.

A tarball has been created in the parent directory.  See
$tarball.
END
exit 0

