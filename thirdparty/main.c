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
#include "../examples/ANSI_RGB_table.c"

int distance_rgb(int i, int r, int g, int b)
{
    int dh = r - rgb256[i][0];
    int ds = g - rgb256[i][1];
    int dl = b - rgb256[i][2];
    return dh*dh + ds*ds + dl*dl;
}

int find_ansi_index_by_rgb(int r, int g, int b)
{
    int index = 0, d_min = distance_rgb(index, r, g, b);
    for (int i = 1; i < 256; ++i) {
        int d = distance_rgb(i, r, g, b);
        if (d < d_min) {
            index = i;
            d_min = d;
        }
    }
    return index;
}
int main()
{
    // int ok = stbi_write_png("imgs/image.png", 3, 3, 4, pixels, 3*sizeof(uint32_t));
    // assert(ok);

    for (int r = 0; r < 5; r++) {
        for (int g = 0; g < 5; g++) {
            for (int b = 0; b < 5; b++) {
                printf("\e[48;5;%dm ", find_ansi_index_by_rgb(255*r/5, 255*g/5, 255*b/5));
            }
            printf("\e[0m\n");
        }
        printf("\e[0m\n");
    }

#if 0
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            printf("\e[48;5;%dm  ", i*16 + j);
        }
        printf("\e[0m\n");
    }
#endif
    return 0;
}