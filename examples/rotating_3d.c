/*
    This example renders a rotating dot matrix with 3D rendering.
    This idea is that you can take this code and compile it to different platforms with different rendering machanisms:
    native with SDL, WebAssembly with HTML5 canvas, etc.
*/
#define OLIVEC_IMPLEMENTATION
#include "olive.c"

float sqrtf(float x);
float atan2f(float y, float x);
float sinf(float x);
float cosf(float x);
#define PI 3.14159265359

#define WIDTH 800
#define HEIGHT 600
#define SCALE_DOWN_FACTOR 10
#define BACKGROUND_COLOR 0xFF181818
#define CIRCLE_COLOR 0xFF2020AA
#define PAGS 8
#define COLS 8
#define ROWS 8
#define GRID_INTERVAL 0.1
#define CIRCLE_RADIUS 5
#define Z_OFFSET 0.5

static uint32_t pixels[WIDTH*HEIGHT];

// TODO: Specify the origin, axis, and angle to achieve spatial rotation
static inline void rotate_point(float *x, float *y, float cx, float cy, float beta)
{
    float xt = *x - cx;
    float yt = *y - cy;

    *x = cosf(beta)*xt - sinf(beta)*yt + cx;
    *y = sinf(beta)*xt + cosf(beta)*yt + cy;
}

void init(void) {}

float theta = 0;

uint32_t *render(float dt)
{
    theta += dt*PI*0.25;

    Olivec_Canvas oc = olivec_canvas(pixels, WIDTH, HEIGHT, WIDTH);
    olivec_fill(oc, BACKGROUND_COLOR);

    for (int k = 0; k < PAGS; k++) {
        for (int i = 0; i < COLS; i++) {
            for (int j = 0; j < ROWS; j++) {
                float x = i*GRID_INTERVAL - (COLS-1)*GRID_INTERVAL/2;
                float y = j*GRID_INTERVAL - (ROWS-1)*GRID_INTERVAL/2;
                float z = k*GRID_INTERVAL + Z_OFFSET;

                float cx = 0;
                float cy = 0;
                float cz = Z_OFFSET + (PAGS - 1)*GRID_INTERVAL/2;
                
                rotate_point(&x, &y, cx, cy, theta);
                rotate_point(&y, &z, cy, cz, theta);
                rotate_point(&x, &z, cx, cz, theta);

                uint32_t r = i*255/COLS;
                uint32_t g = j*255/ROWS;
                uint32_t b = k*255/PAGS;
                uint32_t color = 0xFF000000 | (r<<(0*8)) | (g<<(1*8)) | (b<<(2*8));

                olivec_fill_circle(oc, x/z*HEIGHT/2 + WIDTH/2, (y/z + 1)*HEIGHT/2, CIRCLE_RADIUS, color);                
            }
        }
    }
    return pixels;
}

#include "vc.c"