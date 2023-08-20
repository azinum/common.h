#!/bin/sh

CC="clang"
FLAGS="-pedantic -Wall"

set -xe

${CC} test.c -o test ${FLAGS} -O2
${CC} test.c -o test.wasm ${FLAGS} -Os -nostdlib --target=wasm32 -fno-builtin -Wl,--export-all -Wl,--no-entry -Wl,--allow-undefined -DTARGET_WASM
