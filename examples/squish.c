#define OLIVEC_IMPLEMENTATION
#include "./olive.c"

#include "./assets/kun.c"

#define WIDTH 800
#define HEIGHT 600
#define SCALE_DOWN_FACTOR 10

float sinf(float);

uint32_t dst[WIDTH*HEIGHT];
float global_time = 0;

void init(void){}

#define SRC_SCALE 3

uint32_t *render(float dt)
{
    global_time += dt;

    float t = sinf(0.8*global_time);

    Olivec_Canvas dst_canvas = olivec_canvas(dst, WIDTH, HEIGHT, WIDTH);
    olivec_fill(dst_canvas, 0xFF181818);

    int factor = 100;
    int w = png_width*SRC_SCALE - t*factor;
    int h = png_height*SRC_SCALE + t*factor;

    olivec_copy(
        olivec_subcanvas(dst_canvas, WIDTH/2 - w/2, HEIGHT - h, w, h),
        olivec_canvas(png, png_width, png_height, png_width));

    return dst;
}

#include "vc.c"