#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "../olive.c"

#define WIDTH  40
#define HEIGHT 30
uint32_t pixels[WIDTH*HEIGHT] = {0};

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

int find_ansi_index(uint32_t color)
{
    int r = OLIVEC_RED  (color);
    int g = OLIVEC_GREEN(color);
    int b = OLIVEC_BLUE (color);
    // int a = OLIVEC_ALPHA(color);
    return find_ansi_index_by_rgb(r, g, b);
}

uint32_t mix_color2(float x, uint32_t c1, float x1, uint32_t c2, float x2)
{
    int det = x1 - x2;
    if (OLIVEC_ABS(float, det) < 1.0e-6) return c1;

    int r1 = OLIVEC_RED  (c1);
    int g1 = OLIVEC_GREEN(c1);
    int b1 = OLIVEC_BLUE (c1);
    int a1 = OLIVEC_ALPHA(c1);

    int r2 = OLIVEC_RED  (c2);
    int g2 = OLIVEC_GREEN(c2);
    int b2 = OLIVEC_BLUE (c2);
    int a2 = OLIVEC_ALPHA(c2);

    int u1  = x  - x2;
    int u2  = x1 - x;
    int r = (r1*u1 + r2*u2)/det;
    int g = (g1*u1 + g2*u2)/det;
    int b = (b1*u1 + b2*u2)/det;
    int a = (a1*u1 + a2*u2)/det;

    return OLIVEC_RGBA(r, g, b, a);
}


int main()
{
    // 
    int r = 0, g = 0, b = 0, a = 0xFF;
    for (int y = 0; y < HEIGHT; y+=2) {
        b = 255*y/HEIGHT;
        for (int x = 0; x < WIDTH; x+=2) {
            r = 255*x/WIDTH;
            pixels[y*WIDTH + x] = OLIVEC_RGBA(r, g, b, a);
        }
    }
#if 0
    // first x then y
    for (int y = 0; y < HEIGHT; y+=2) {
        for (int x = 1; x < WIDTH; x+=2) {
            pixels[y*WIDTH + x] = mix_color2(x, pixels[y*WIDTH + x - 1], x - 1, pixels[y*WIDTH + x + 1], x + 1);
        }
    }
    for (int y = 1; y < HEIGHT; y+=2) {
        for (int x = 0; x < WIDTH; x++) {
            pixels[y*WIDTH + x] = mix_color2(y, pixels[(y - 1)*WIDTH + x], y - 1, pixels[(y + 1)*WIDTH + x], y + 1);
        }
    }
#else
    // first y then x
    for (int y = 1; y < HEIGHT-1; y+=2) {
        for (int x = 0; x < WIDTH; x+=2) {
            pixels[y*WIDTH + x] = mix_color2(y, pixels[(y - 1)*WIDTH + x], y - 1, pixels[(y + 1)*WIDTH + x], y + 1);
        }
    }
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 1; x < WIDTH-1; x+=2) {
            pixels[y*WIDTH + x] = mix_color2(x, pixels[y*WIDTH + x - 1], x - 1, pixels[y*WIDTH + x + 1], x + 1);
        }
    }
#endif
    // output
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            printf("\e[48;5;%dm  ", find_ansi_index(pixels[y*WIDTH + x]));
        }
        printf("\e[0m\n");
    }
#if 1
    int ok = stbi_write_png("./image.png", WIDTH, HEIGHT, 4, pixels, WIDTH*sizeof(uint32_t));
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