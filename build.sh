#!/bin/sh

set -xe

build_vc_example() {
    NAME=$1

    clang -Os -fno-builtin -Wall -Wextra -Wswitch-enum --target=wasm32 --no-standard-libraries -Wl,--no-entry -Wl,--export-all -Wl,--allow-undefined  -o ./bin/$NAME.wasm -I. -DPLATFORM=WASM_PLATFORM ./examples/$NAME.c
    mkdir -p ./wasm/ && cp ./bin/$NAME.wasm ./wasm/
    gcc -O2 -Wall -Wextra -ggdb -I. -I$SDL2_PATH/include -o ./bin/$NAME.sdl  -DPLATFORM=SDL_PLATFORM -DSDL_MAIN_HANDLED ./examples/$NAME.c -lm -L$SDL2_PATH/lib -lSDL2
    gcc -O2 -Wall -Wextra -ggdb -I. -o ./bin/$NAME.term -DPLATFORM=TERM_PLATFORM ./examples/$NAME.c -lm
}

mkdir -p ./bin/
gcc -Wall -Wextra -ggdb -o ./bin/test -Ithirdparty test.c -lm &
gcc -Wall -Wextra -ggdb -o ./bin/gallery -Ithirdparty -I. examples/gallery.c -lm &
gcc -Wall -Wextra -ggdb -o ./bin/png2c -Ithirdparty png2c.c -lm &
wait

# mkdir -p ./build/assets/
./bin/png2c ./assets/tsodinPog.png ./assets/kun.c

build_vc_example triangle &
build_vc_example rotating_3d &
build_vc_example squish &
wait

