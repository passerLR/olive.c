#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

uint32_t pixels[] = {
    0xFF0000FF, 0xFF000000, 0xFF0000FF,
    0xFF000000, 0xFF0000FF, 0xFF000000,
    0xFF0000FF, 0xFF000000, 0xFF0000FF,
};

int main()
{
    int ok = stbi_write_png("imgs/image.png", 3, 3, 4, pixels, 3*sizeof(uint32_t));
    assert(ok);
    return 0;
}