#include <stdio.h>
#include "olive.c"

#define WIDTH 800
#define HEIGHT 600

static uint32_t pixels[HEIGHT*WIDTH];

#define COLS (8*4)
#define ROWS (6*4)
#define CELL_WIDTH  (WIDTH/COLS)
#define CELL_HEIGHT (HEIGHT/ROWS)

#define BACKGROUND_COLOR 0xFF202020
#define FOREGROUND_COLOR 0xFF2020FF

void swap_int(int *a, int *b)
{
    int t = *a;
    *a = *b;
    *b = t;
}

void olivec_draw_line(uint32_t *pixels, size_t pixels_width, size_t pixels_height, 
                      int x1, int y1, int x2, int y2, 
                      uint32_t color)
{
    int dx = x2 - x1;
    int dy = y2 - y1;
    if (dx != 0) {
        int c = y1 - dy*x1/dx;

        if (x1 > x2) swap_int(&x1, &x2);
        for (int x = x1; x <= x2; x++) {
            if (0 <= x && x < (int) pixels_width) {
                int ym = dy*x/dx + c;
                int yp = dy*(x+1)/dx + c;
                if (ym > yp) swap_int(&ym, &yp);
                for(int y = ym; y <= yp; y++) {
                    if (0 <= y && y < (int) pixels_height) {
                        pixels[y*pixels_width + x] = color;
                    }
                }
            }
        }
    } else {
        int x = x1;
        if (0 <= x && x < (int) pixels_width) {
            if (y1 > y2) swap_int(&y1, &y2);
            for (int y = y1; y <= y2; y++) {
                if (0 <= y && y < (int) pixels_height) {
                    pixels[y*pixels_width + x] = color;
                }
            }
        }
    }
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

    const char *file_path = "checker.ppm";
    Errno err = olivec_save_to_ppm_file(pixels, WIDTH, HEIGHT, file_path);
    if (err) {
        fprintf(stderr, "ERROR: could not save file %s: %s\n", file_path, strerror(errno));
        return 1;
    }

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

    const char *file_path = "circle.ppm";
    Errno err = olivec_save_to_ppm_file(pixels, WIDTH, HEIGHT, file_path);
    if (err) {
        fprintf(stderr, "ERROR: could not save file %s: %s\n", file_path, strerror(errno));
        return 1;
    }

    return 0;
}

uint8_t line_example(void)
{
    olivec_fill(pixels, WIDTH, HEIGHT, BACKGROUND_COLOR);
    olivec_draw_line(pixels, WIDTH, HEIGHT, 0, 0, WIDTH, HEIGHT, FOREGROUND_COLOR);
    olivec_draw_line(pixels, WIDTH, HEIGHT, WIDTH, 0, 0, HEIGHT, FOREGROUND_COLOR);
    olivec_draw_line(pixels, WIDTH, HEIGHT, 0, 0, WIDTH/4, HEIGHT, 0x2000FF00);
    olivec_draw_line(pixels, WIDTH, HEIGHT, WIDTH/4, 0, 0, HEIGHT, 0x2000FF00);
    olivec_draw_line(pixels, WIDTH, HEIGHT, 0, HEIGHT/2, WIDTH, HEIGHT/2, 0x20FF3000);
    olivec_draw_line(pixels, WIDTH, HEIGHT, WIDTH/2, 0, WIDTH/2, HEIGHT,  0x20FF3000);

    const char *file_path = "line.ppm";
    Errno err = olivec_save_to_ppm_file(pixels, WIDTH, HEIGHT, file_path);
    if (err) {
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
    return 0;
} 
