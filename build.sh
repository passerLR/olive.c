#!/bin/sh

set -xe

CC=gcc
EMCC=clang
COMMON_CFLAGS="-Wall -Wextra -std=c99 -ggdb -I. -I./thirdparty/"

build_wasm_demo() {
    NAME=$1
    $EMCC $COMMON_CFLAGS -O2 -fno-builtin --target=wasm32 --no-standard-libraries -Wl,--no-entry -Wl,--export=render -Wl,--export=__heap_base -Wl,--allow-undefined  -o ./bin/$NAME.wasm -DPLATFORM=WASM_PLATFORM ./examples/$NAME.c
    mkdir -p ./wasm/ && cp ./bin/$NAME.wasm ./wasm/
}

build_term_demo() {
    NAME=$1
    $CC $COMMON_CFLAGS -O2 -o ./bin/$NAME.term -DPLATFORM=TERM_PLATFORM ./examples/$NAME.c -lm
}

build_sdl_demo() {
    NAME=$1
    $CC $COMMON_CFLAGS -O2 -I$SDL2_PATH/include -o ./bin/$NAME.sdl -DPLATFORM=SDL_PLATFORM -DSDL_MAIN_HANDLED ./examples/$NAME.c -lm -L$SDL2_PATH/lib -lSDL2
}

build_vc_demo() {
    NAME=$1
    build_wasm_demo $NAME
    build_term_demo $NAME
    build_sdl_demo $NAME
}

build_all_vc_demos() {
    mkdir -p ./bin
    build_vc_demo triangle &
    build_vc_demo rotating_3d &
    build_vc_demo squish &
    build_vc_demo triangle_3d &
    build_vc_demo triangle_texture &
    build_vc_demo cup3d &
    build_vc_demo teapot3d &
    wait # TODO: the whole script must fail if one of the jobs fails
}

build_tools() {
    mkdir -p ./bin
    $CC $COMMON_CFLAGS -O2 -o ./bin/png2c ./tools/png2c.c -lm &
    $CC $COMMON_CFLAGS -O2 -o ./bin/obj2c ./tools/obj2c.c -lm &
    wait # TODO: the whole script must fail if one of the jobs fails
}

build_assets() {
    mkdir -p ./assets/
    ./bin/png2c -n tsodinPog -o ./assets/tsodinPog.c ./assets/ppng.png &
    # ./bin/png2c -n Sadge -o ./assets/Sadge.c ./assets/Sadge.png &
    ./bin/png2c -n kun -o ./assets/kun.c ./assets/kun.png&
    ./bin/obj2c ./assets/cup.obj -o ./assets/cup.c &
    ./bin/obj2c ./assets/teapot.obj -o ./assets/teapot.c -s 0.40 &
    wait # TODO: the whole script must fail if one of the jobs fails
}

build_tests() {
    $CC $COMMON_CFLAGS -o ./bin/test ./test.c -lm &
}

build_tools
build_assets
build_tests
build_all_vc_demos