#!/bin/sh

CC="gcc"
FLAGS="-Wall"
NPROC=`nproc`
CACHELINESIZE=`getconf LEVEL1_DCACHE_LINESIZE`

set -xe

${CC} test.c -o test ${FLAGS} -O2 -DNPROC=${NPROC} -DCACHELINESIZE=${CACHELINESIZE} -lpthread
which x86_64-w64-mingw32-gcc 1>/dev/null && CC=x86_64-w64-mingw32-gcc && \
${CC} test.c -o test ${FLAGS} -O2 -DNPROC=${NPROC} -DCACHELINESIZE=${CACHELINESIZE} -lpthread
