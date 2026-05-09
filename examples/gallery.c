#include <stdio.h>
#include <assert.h>
#include <errno.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define OLIVEC_IMPLEMENTATION
#include "olive.c"

#define WIDTH  800
#define HEIGHT 600

static uint32_t pixels[HEIGHT*WIDTH];

#define COLS (8*2)
#define ROWS (6*2)
#define CELL_WIDTH  (WIDTH/COLS)
#define CELL_HEIGHT (HEIGHT/ROWS)

#define BACKGROUND_COLOR 0xFF202020
#define FOREGROUND_COLOR 0xFF0000FF

#define IMG_DIR_PATH "./imgs"

uint8_t triangle_example(void)
{
    Olivec_Canvas oc = olivec_canvas(pixels, WIDTH, HEIGHT);
    olivec_fill(oc, BACKGROUND_COLOR);
    {
        int x1 = WIDTH/2, y1 = HEIGHT/8;
        int x2 = WIDTH/8, y2 = HEIGHT/2;
        int x3 = WIDTH*7/8, y3 = HEIGHT*7/8;
        uint32_t color = 0xFF0000FF;
        olivec_fill_triangle(oc, x1, y1, x2, y2, x3, y3, color);
    }
    {
        int x1 = WIDTH*5/8, y1 = HEIGHT/8;
        int x2 = WIDTH*3/8, y2 = HEIGHT/2;
        int x3 = WIDTH*7/8, y3 = HEIGHT/2;
        uint32_t color = 0xFF00FF00;
        olivec_fill_triangle(oc, x1, y1, x2, y2, x3, y3, color);
    }
    {
        int x1 = WIDTH*3/8, y1 = HEIGHT/8;
        int x2 = WIDTH*3/8, y2 = HEIGHT*3/8;
        int x3 = WIDTH*5/8, y3 = HEIGHT*3/8;
        uint32_t color = 0xFFFF0000;
        olivec_fill_triangle(oc, x1, y1, x2, y2, x3, y3, color);
    }
    {
        int x1 = WIDTH*2/8, y1 = HEIGHT/8;
        int x2 = WIDTH*2/8, y2 = HEIGHT*3/8;
        int x3 = WIDTH*1/8, y3 = HEIGHT*3/8;
        uint32_t color = 0xFFFFFFFF;
        olivec_fill_triangle(oc, x1, y1, x2, y2, x3, y3, color);
    }

    const char *file_path = IMG_DIR_PATH"/triangle.png";
    printf("Generated %s\n", file_path);
    if (!stbi_write_png(file_path, WIDTH, HEIGHT, 4, pixels, WIDTH*sizeof(uint32_t))) {
        fprintf(stderr, "ERROR: could not save file %s: %s\n", file_path, strerror(errno));
        return 1;
    }

    return 0;
}

uint8_t checker_example(void)
{
    Olivec_Canvas oc = olivec_canvas(pixels, WIDTH, HEIGHT);
    olivec_fill(oc, BACKGROUND_COLOR);

    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
            uint32_t color;
            if ((x + y)%2 == 0) {
                color = BACKGROUND_COLOR;
            } else {
                color = FOREGROUND_COLOR;
            }
            olivec_fill_rect(oc, x*CELL_WIDTH, y*CELL_HEIGHT, CELL_WIDTH, CELL_HEIGHT, color);
        }
    }

    const char *file_path = IMG_DIR_PATH"/checker.png";
    printf("Generated %s\n", file_path);
    if (!stbi_write_png(file_path, WIDTH, HEIGHT, 4, pixels, WIDTH*sizeof(uint32_t))) {
        fprintf(stderr, "ERROR: could not save file %s: %s\n", file_path, strerror(errno));
        return 1;
    }

    return 0;
}

uint8_t circle_example(void)
{
    Olivec_Canvas oc = olivec_canvas(pixels, WIDTH, HEIGHT);
    olivec_fill(oc, BACKGROUND_COLOR);

    int radius = CELL_WIDTH < CELL_HEIGHT ? CELL_WIDTH : CELL_HEIGHT;
    radius >>= 1;
    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
            // t = (x/(COLS*2) + y/(ROWS*2))
            // c = 4
            // radius/c + (radius - radius/c)*t
            // radius/c + (c - 1)/c*radius*t
            // radius*(1 + (c - 1)*t)/4
            olivec_fill_circle(oc, x*CELL_WIDTH + CELL_WIDTH/2, y*CELL_HEIGHT + CELL_HEIGHT/2, -radius/8*(1 + 7*(x/(COLS*2.0) + y/(ROWS*2.0))), FOREGROUND_COLOR);
        }
    }

    const char *file_path = IMG_DIR_PATH"/circle.png";
    printf("Generated %s\n", file_path);
    if (!stbi_write_png(file_path, WIDTH, HEIGHT, 4, pixels, WIDTH*sizeof(uint32_t))) {
        fprintf(stderr, "ERROR: could not save file %s: %s\n", file_path, strerror(errno));
        return 1;
    }

    return 0;
}

uint8_t line_example(void)
{
    Olivec_Canvas oc = olivec_canvas(pixels, WIDTH, HEIGHT);

    olivec_fill(oc, BACKGROUND_COLOR);
    olivec_draw_line(oc, 0, 0, WIDTH, HEIGHT, FOREGROUND_COLOR);
    olivec_draw_line(oc, WIDTH, 0, 0, HEIGHT, FOREGROUND_COLOR);
    olivec_draw_line(oc, 0, 0, WIDTH/4, HEIGHT, 0xFF00FF00);
    olivec_draw_line(oc, WIDTH/4, 0, 0, HEIGHT, 0xFF00FF00);
    olivec_draw_line(oc, 0, HEIGHT/2, WIDTH, HEIGHT/2, 0xFFFF0000);
    olivec_draw_line(oc, WIDTH/2, 0, WIDTH/2, HEIGHT,  0xFFFF0000);

    const char *file_path = IMG_DIR_PATH"/line.png";
    printf("Generated %s\n", file_path);
    if (!stbi_write_png(file_path, WIDTH, HEIGHT, 4, pixels, WIDTH*sizeof(uint32_t))) {
        fprintf(stderr, "ERROR: could not save file %s: %s\n", file_path, strerror(errno));
        return 1;
    }

    return 0;
}

uint8_t rect_example(void)
{
    Olivec_Canvas oc = olivec_canvas(pixels, WIDTH, HEIGHT);

    olivec_fill(oc, BACKGROUND_COLOR);
    olivec_fill_rect(oc, 0, 0, WIDTH/2, HEIGHT/2, FOREGROUND_COLOR);
    olivec_fill_rect(oc, WIDTH-1, HEIGHT-1, -WIDTH/2, -HEIGHT/2, 0xFF00FF00);

    const char *file_path = IMG_DIR_PATH"/rect.png";
    printf("Generated %s\n", file_path);
    if (!stbi_write_png(file_path, WIDTH, HEIGHT, 4, pixels, WIDTH*sizeof(uint32_t))) {
        fprintf(stderr, "ERROR: could not save file %s: %s\n", file_path, strerror(errno));
        return 1;
    }

    return 0;
}

uint8_t text_example(void)
{
    Olivec_Canvas oc = olivec_canvas(pixels, WIDTH, HEIGHT);

    olivec_fill(oc, BACKGROUND_COLOR);
    const char text[] = "the quick brown fox jumps over the lazy dog.";
    const char lowercase[] = "abcdefghijklmnopqrstuvwxyz";
    const char uppercase[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const char number[]    = "01234567890123456789";
    const char symbols1[]  = ". , : ; \" \' ! ? + - * / % = ~ |";
    const char symbols2[]  = "@ # $ _ ^ \\ & < > ( ) [ ] { }";
    olivec_text(oc, text, default_font, 0, 0, 2, 0xFFFFFFFF);
    olivec_text(oc, lowercase, default_font, 0, HEIGHT*1/8, 3, 0xFFFFFFFF);
    olivec_text(oc, uppercase, default_font, 0, HEIGHT*2/8, 3, 0xFFFFFFFF);
    olivec_text(oc, number,    default_font, 0, HEIGHT*3/8, 3, 0xFFFFFFFF);
    olivec_text(oc, symbols1,  default_font, 0, HEIGHT*4/8, 3, 0xFFFFFFFF);
    olivec_text(oc, symbols2,  default_font, 0, HEIGHT*5/8, 3, 0xFFFFFFFF);

    const char *file_path = IMG_DIR_PATH"/text.png";
    printf("Generated %s\n", file_path);
    if (!stbi_write_png(file_path, WIDTH, HEIGHT, 4, pixels, WIDTH*sizeof(uint32_t))) {
        fprintf(stderr, "ERROR: could not save file %s: %s\n", file_path, strerror(errno));
        return 1;
    }

    return 0;
}

int main(void)
{
    if (checker_example()) return -1;
    if (circle_example()) return -1;
    if (line_example()) return -1;
    if (triangle_example()) return -1;
    if (rect_example()) return -1;
    if (text_example()) return -1;
    return 0;
} 

