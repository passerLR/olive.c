#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define OLIVEC_IMPLEMENTATION
#include "../olive.c"

#define WIDTH 800
#define HEIGHT 600
#define BACKGROUND_COLOR 0xFF181818

static uint32_t pixels[WIDTH*HEIGHT];
static float triangle_angle = 0;

int main()
{
    Olivec_Canvas oc = olivec_canvas(pixels, WIDTH, HEIGHT, WIDTH);
    olivec_fill(oc, BACKGROUND_COLOR);

    // Triangle
    {
        // triangle_angle += 0.5f*PI*dt;
        triangle_angle = 0.5f*PI*1.0;

        float x1 = WIDTH/2, y1 = HEIGHT/8;
        float x2 = WIDTH/8, y2 = HEIGHT/2;
        float x3 = WIDTH*7/8, y3 = HEIGHT*7/8;
        rotate_point(&x1, &y1, WIDTH/2, HEIGHT/2, triangle_angle);
        rotate_point(&x2, &y2, WIDTH/2, HEIGHT/2, triangle_angle);
        rotate_point(&x3, &y3, WIDTH/2, HEIGHT/2, triangle_angle);
        olivec_fill_triangle3c(oc, x1, y1, x2, y2, x3, y3, 0xFF2020AA, 0xFF20AA20, 0xFFAA2020);
    }
#if 1
    int ok = stbi_write_png("./image.png", WIDTH, HEIGHT, 4, oc.pixels, WIDTH*sizeof(uint32_t));
    assert(ok);
#endif

#if 0
    for (int r = 0; r < 5; r++) {
        for (int g = 0; g < 5; g++) {
            for (int b = 0; b < 5; b++) {
                printf("\e[48;5;%dm ", find_ansi_index_by_rgb(255*r/5, 255*g/5, 255*b/5));
            }
            printf("\e[0m\n");
        }
        printf("\e[0m\n");
    }

    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            printf("\e[48;5;%dm  ", i*16 + j);
        }
        printf("\e[0m\n");
    }
#endif
    return 0;
}
