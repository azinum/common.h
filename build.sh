#!/bin/sh

CC="clang"
FLAGS="-pedantic -Wall"
NPROC=`nproc`
CACHELINESIZE=`getconf LEVEL1_DCACHE_LINESIZE`

set -xe

${CC} test.c -o test ${FLAGS} -O2 -DNPROC=${NPROC} -DCACHELINESIZE=${CACHELINESIZE}
${CC} test.c -o test.wasm ${FLAGS} -Os -nostdlib --target=wasm32 -fno-builtin -Wl,--export-all -Wl,--no-entry -Wl,--allow-undefined -DTARGET_WASM -DNO_STDLIB -DNO_STDIO
