#!/bin/sh

set -xe

COMMON_CFLAGS="-Wall -Wextra -ggdb -I. -I./thirdparty/"

build_vc_example() {
    NAME=$1

    clang $COMMON_CFLAGS -Os -fno-builtin -Wswitch-enum --target=wasm32 --no-standard-libraries -Wl,--no-entry -Wl,--export=render -Wl,--export=__heap_base -Wl,--allow-undefined  -o ./bin/$NAME.wasm -DPLATFORM=WASM_PLATFORM ./examples/$NAME.c
    mkdir -p ./wasm/ && cp ./bin/$NAME.wasm ./wasm/
    gcc -O2 $COMMON_CFLAGS -I$SDL2_PATH/include -o ./bin/$NAME.sdl -DPLATFORM=SDL_PLATFORM -DSDL_MAIN_HANDLED ./examples/$NAME.c -lm -L$SDL2_PATH/lib -lSDL2
    gcc -O2 $COMMON_CFLAGS -o ./bin/$NAME.term -DPLATFORM=TERM_PLATFORM ./examples/$NAME.c -lm
}

mkdir -p ./bin/
gcc $COMMON_CFLAGS -o ./bin/test  test.c -lm

gcc $COMMON_CFLAGS -o ./bin/viewobj  viewobj.c -lm

gcc $COMMON_CFLAGS -o ./bin/png2c png2c.c -lm
# mkdir -p ./build/assets/
./bin/png2c ./assets/tsodinPog.png -o ./assets/ppng.c
./bin/png2c -n kun -o ./assets/kun.c ./assets/kun.png

build_vc_example triangle &
build_vc_example rotating_3d &
build_vc_example squish &
build_vc_example triangle_3d &
build_vc_example triangle_texture &
wait
