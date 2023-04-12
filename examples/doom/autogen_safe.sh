#!/bin/sh

# this script builds chocolate doom with our clang build

CLANG_PATH=../../build/bin/clang
# resolve absolute path
export CC=`readlink -f $CLANG_PATH`
cd ./chocolate-doom
autoreconf -fi
./configure "$@"
