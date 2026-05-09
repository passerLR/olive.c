#!/bin/sh

set -xe

mkdir -p ./bin/
# gcc -Wall -Wextra -ggdb -o ./bin/test -Ithirdparty test.c -lm
# gcc -Wall -Wextra -ggdb -o ./bin/gallery -Ithirdparty -I. examples/gallery.c
# #emcc -Os -fno-builtin -Wall -Wextra -Wswitch-enum --target=wasm32 --no-standard-libraries -Wl,--no-entry -Wl,--export-all -Wl,--allow-undefined  -o ./bin/triangle.wasm -I. ./examples/triangle.c
# # gcc -Wall -Wextra -ggdb -I. -DSDL_PLATFORM -o ./bin/triangle ./examples/triangle.c -lm -lSDL2

# ## build command for Windows
# clang -Os -fno-builtin -Wall -Wextra -Wswitch-enum --target=wasm32 --no-standard-libraries -Wl,--no-entry -Wl,--export-all -Wl,--allow-undefined  -o ./bin/triangle.wasm -I. ./examples/triangle.c
# gcc -Wall -Wextra -ggdb -I. -DSDL_MAIN_HANDLED -DSDL_PLATFORM -o ./bin/triangle -I$SDL2_PATH/include ./examples/triangle.c -lm -L$SDL2_PATH/lib -lSDL2 -mwindows

clang -Os -fno-builtin -Wall -Wextra -Wswitch-enum --target=wasm32 --no-standard-libraries -Wl,--no-entry -Wl,--export-all -Wl,--allow-undefined  -o ./bin/rotating_3d.wasm -I. ./examples/rotating_3d.c
gcc -Wall -Wextra -ggdb -I. -DSDL_MAIN_HANDLED -DSDL_PLATFORM -o ./bin/rotating_3d -I$SDL2_PATH/include ./examples/rotating_3d.c -lm -L$SDL2_PATH/lib -lSDL2 -mwindows
