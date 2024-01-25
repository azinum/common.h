#!/bin/sh

CC="gcc"
FLAGS="-Wall"
NPROC=`nproc`
CACHELINESIZE=`getconf LEVEL1_DCACHE_LINESIZE`

set -xe

${CC} test.c -o test ${FLAGS} -O2 -DNPROC=${NPROC} -DCACHELINESIZE=${CACHELINESIZE} -lpthread
