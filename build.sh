#!/bin/sh

CC="clang"
FLAGS="-O2 -pedantic"

set -xe

${CC} test.c -o test ${FLAGS}
