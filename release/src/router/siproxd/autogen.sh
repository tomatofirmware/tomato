#!/bin/sh
# Run this to generate all the initial makefiles, etc.

test -f configure.in || {
    echo "**Error**: This directory does not look like the top-level directory"
    exit 1
}

set -e
aclocal
autoheader
libtoolize --ltdl --copy --force
automake --add-missing --copy
autoconf
