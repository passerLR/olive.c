#include <stdio.h>
#include <assert.h>
#include "olive.c"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define WIDTH 800
#define HEIGHT 600

static uint32_t pixels[HEIGHT*WIDTH];

#define COLS (8*2)
#define ROWS (6*2)
#define CELL_WIDTH  (WIDTH/COLS)
#define CELL_HEIGHT (HEIGHT/ROWS)

#define BACKGROUND_COLOR 0xFF202020
#define FOREGROUND_COLOR 0xFF0000FF

uint8_t triangle_example(void)
{
    olivec_fill(pixels, WIDTH, HEIGHT, BACKGROUND_COLOR);
    {
        int x1 = WIDTH/2, y1 = HEIGHT/8;
        int x2 = WIDTH/8, y2 = HEIGHT/2;
        int x3 = WIDTH*7/8, y3 = HEIGHT*7/8;
        uint32_t color = 0xFF0000FF;
        olive_fill_triangle(pixels, WIDTH, HEIGHT, x1, y1, x2, y2, x3, y3, color);
    }
    {
        int x1 = WIDTH*5/8, y1 = HEIGHT/8;
        int x2 = WIDTH*3/8, y2 = HEIGHT/2;
        int x3 = WIDTH*7/8, y3 = HEIGHT/2;
        uint32_t color = 0xFF00FF00;
        olive_fill_triangle(pixels, WIDTH, HEIGHT, x1, y1, x2, y2, x3, y3, color);
    }
    {
        int x1 = WIDTH*3/8, y1 = HEIGHT/8;
        int x2 = WIDTH*3/8, y2 = HEIGHT*3/8;
        int x3 = WIDTH*5/8, y3 = HEIGHT*3/8;
        uint32_t color = 0xFFFF0000;
        olive_fill_triangle(pixels, WIDTH, HEIGHT, x1, y1, x2, y2, x3, y3, color);
    }
    {
        int x1 = WIDTH*2/8, y1 = HEIGHT/8;
        int x2 = WIDTH*2/8, y2 = HEIGHT*3/8;
        int x3 = WIDTH*1/8, y3 = HEIGHT*3/8;
        uint32_t color = 0xFFFFFFFF;
        olive_fill_triangle(pixels, WIDTH, HEIGHT, x1, y1, x2, y2, x3, y3, color);
    }

    // const char *file_path = "triangle.ppm";
    // Errno err = olivec_save_to_ppm_file(pixels, WIDTH, HEIGHT, file_path);
    // if (err) {
    //     fprintf(stderr, "ERROR: could not save file %s: %s\n", file_path, strerror(errno));
    //     return 1;
    // }

    const char *file_path = "imgs/triangle.png";
    int ok = stbi_write_png(file_path, WIDTH, HEIGHT, 4, pixels, WIDTH*sizeof(uint32_t));
    assert(ok);

    return 0;
}

uint8_t checker_example(void)
{
    olivec_fill(pixels, WIDTH, HEIGHT, BACKGROUND_COLOR);
    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
            uint32_t color;
            if ((x + y)%2 == 0) {
                color = BACKGROUND_COLOR;
            } else {
                color = FOREGROUND_COLOR;
            }
            olivec_fill_rect(pixels, WIDTH, HEIGHT, x*CELL_WIDTH, y*CELL_HEIGHT, CELL_WIDTH, CELL_HEIGHT, color);
        }
    }

    // const char *file_path = "checker.ppm";
    // Errno err = olivec_save_to_ppm_file(pixels, WIDTH, HEIGHT, file_path);
    // if (err) {
    //     fprintf(stderr, "ERROR: could not save file %s: %s\n", file_path, strerror(errno));
    //     return 1;
    // }

    const char *file_path = "imgs/checker.png";
    int ok = stbi_write_png(file_path, WIDTH, HEIGHT, 4, pixels, WIDTH*sizeof(uint32_t));
    assert(ok);

    return 0;
}

uint8_t circle_example(void)
{
    olivec_fill(pixels, WIDTH, HEIGHT, BACKGROUND_COLOR);
    size_t radius = CELL_WIDTH < CELL_HEIGHT ? CELL_WIDTH : CELL_HEIGHT;
    radius >>= 1;
    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
            // t = (x/(COLS*2) + y/(ROWS*2))
            // c = 4
            // radius/c + (radius - radius/c)*t
            // radius/c + (c - 1)/c*radius*t
            // radius*(1 + (c - 1)*t)/4
            olivec_fill_circle(pixels, WIDTH, HEIGHT, x*CELL_WIDTH + CELL_WIDTH/2, y*CELL_HEIGHT + CELL_HEIGHT/2, radius/8*(1 + 7*(x/(COLS*2.0) + y/(ROWS*2.0))), FOREGROUND_COLOR);
        }
    }

    // const char *file_path = "circle.ppm";
    // Errno err = olivec_save_to_ppm_file(pixels, WIDTH, HEIGHT, file_path);
    // if (err) {
    //     fprintf(stderr, "ERROR: could not save file %s: %s\n", file_path, strerror(errno));
    //     return 1;
    // }
    
    const char *file_path = "imgs/circle.png";
    int ok = stbi_write_png(file_path, WIDTH, HEIGHT, 4, pixels, WIDTH*sizeof(uint32_t));
    assert(ok);

    return 0;
}

uint8_t line_example(void)
{
    olivec_fill(pixels, WIDTH, HEIGHT, BACKGROUND_COLOR);
    olivec_draw_line(pixels, WIDTH, HEIGHT, 0, 0, WIDTH, HEIGHT, FOREGROUND_COLOR);
    olivec_draw_line(pixels, WIDTH, HEIGHT, WIDTH, 0, 0, HEIGHT, FOREGROUND_COLOR);
    olivec_draw_line(pixels, WIDTH, HEIGHT, 0, 0, WIDTH/4, HEIGHT, 0xFF00FF00);
    olivec_draw_line(pixels, WIDTH, HEIGHT, WIDTH/4, 0, 0, HEIGHT, 0xFF00FF00);
    olivec_draw_line(pixels, WIDTH, HEIGHT, 0, HEIGHT/2, WIDTH, HEIGHT/2, 0xFFFF0000);
    olivec_draw_line(pixels, WIDTH, HEIGHT, WIDTH/2, 0, WIDTH/2, HEIGHT,  0xFFFF0000);

    // const char *file_path = "line.ppm";
    // Errno err = olivec_save_to_ppm_file(pixels, WIDTH, HEIGHT, file_path);
    // if (err) {
    //     fprintf(stderr, "ERROR: could not save file %s: %s\n", file_path, strerror(errno));
    //     return 1;
    // }

    const char *file_path = "imgs/line.png";
    int ok = stbi_write_png(file_path, WIDTH, HEIGHT, 4, pixels, WIDTH*sizeof(uint32_t));
    assert(ok);

    return 0;
}

int main(void)
{
    if (checker_example()) return -1;
    if (circle_example()) return -1;
    if (line_example()) return -1;
    if (triangle_example()) return -1;
    return 0;
} 
