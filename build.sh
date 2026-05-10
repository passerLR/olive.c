#!/bin/sh

set -xe

mkdir -p ./bin/
# gcc -Wall -Wextra -ggdb -o ./bin/test -Ithirdparty test.c -lm
# gcc -Wall -Wextra -ggdb -o ./bin/gallery -Ithirdparty -I. examples/gallery.c

clang -Os -fno-builtin -Wall -Wextra -Wswitch-enum --target=wasm32 --no-standard-libraries -Wl,--no-entry -Wl,--export-all -Wl,--allow-undefined  -o ./bin/triangle.wasm -I. -DPLATFORM=WASM_PLATFORM ./examples/triangle.c
gcc -O2 -Wall -Wextra -ggdb -I. -I$SDL2_PATH/include -o ./bin/triangle.sdl  -DPLATFORM=SDL_PLATFORM -DSDL_MAIN_HANDLED ./examples/triangle.c -lm -L$SDL2_PATH/lib -lSDL2
gcc -O2 -Wall -Wextra -ggdb -I. -o ./bin/triangle.term -DPLATFORM=TERM_PLATFORM ./examples/triangle.c -lm

clang -Os -fno-builtin -Wall -Wextra -Wswitch-enum --target=wasm32 --no-standard-libraries -Wl,--no-entry -Wl,--export-all -Wl,--allow-undefined  -o ./bin/rotating_3d.wasm -I. -DPLATFORM=WASM_PLATFORM ./examples/rotating_3d.c
gcc -O2 -Wall -Wextra -ggdb -I. -I$SDL2_PATH/include -o ./bin/rotating_3d.sdl -DPLATFORM=SDL_PLATFORM -DSDL_MAIN_HANDLED ./examples/rotating_3d.c -lm -L$SDL2_PATH/lib -lSDL2
gcc -O2 -Wall -Wextra -ggdb -I. -o ./bin/rotating_3d.term -DPLATFORM=TERM_PLATFORM ./examples/rotating_3d.c -lm