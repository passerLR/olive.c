#!/bin/sh

set -xe

mkdir -p ./bin/
gcc -Wall -Wextra -ggdb -o ./bin/test -Ithirdparty test.c -lm
gcc -Wall -Wextra -ggdb -o ./bin/gallery -Ithirdparty -I. examples/gallery.c
emcc -Os -fno-builtin -Wall -Wextra -Wswitch-enum --target=wasm32 --no-standard-libraries -Wl,--no-entry -Wl,--export-all -Wl,--allow-undefined  -o ./bin/triangle.wasm -I. ./examples/triangle.c
gcc -Wall -Wextra -ggdb -I. -DSDL_PLATFORM -o ./bin/triangle ./examples/triangle.c -lm -lSDL2

