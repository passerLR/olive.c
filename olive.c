#pragma once

#include <stdint.h>

#define OLIVE_SWAP(T, a, b) do {T t = a; a = b; b = t;} while (0)
#define OLIVE_SIGN(T, x) ((T)((x) > 0) - (T)((x) < 0))
#define OLIVE_ABS(T, x) (OLIVE_SIGN(T, x)*(x))


void olivec_fill(uint32_t *pixels, size_t width, size_t height, uint32_t color)
{
    for (size_t i = 0; i < width*height; i++) {
        pixels[i] = color;
    }
}

/*
Errno olivec_save_to_ppm_file(uint32_t *pixels, size_t width, size_t height, const char *file_path)
{
    int result = 0;
    FILE *f  = NULL;
    {
        f = fopen(file_path, "wb");
        if (f == NULL) return_defer(errno);

        fprintf(f, "P6\n%zu %zu 255\n", width, height);
        if(ferror(f)) return_defer(errno);

        for (size_t i = 0; i < width*height; i++) {
            // 0xAABBGGRR
            uint32_t pixel = pixels[i];
            uint8_t  bytes[3] = {
                (pixel>>(8*0))&0xFF,
                (pixel>>(8*1))&0xFF,
                (pixel>>(8*2))&0xFF,
            };
            fwrite(bytes, sizeof(bytes), 1, f);
            if(ferror(f)) return_defer(errno);
        }
    }

defer:
    if (f) fclose(f);
    return result;
}
*/

void olivec_fill_rect(uint32_t *pixels, size_t pixels_width, size_t pixels_height, 
                      int x0, int y0, int w, int h, 
                      uint32_t color)
{
    int xt = x0 + OLIVE_SIGN(int, w)*(OLIVE_ABS(int, w) - 1);
    if (xt < x0) OLIVE_SWAP(int, xt, x0);
    int yt = y0 + OLIVE_SIGN(int, h)*(OLIVE_ABS(int, h) - 1);
    if (yt < y0) OLIVE_SWAP(int, yt, y0);

    for (int y = y0; y <= yt; ++y) {
        // TODO: move boundaries check out of olivec_fill_rect
        if (0 <= y && y < (int) pixels_height) {
            for (int x = x0; x <= xt; ++x) {
                if (0 <= x && x < (int) pixels_width) {
                    pixels[y*pixels_width + x] = color;
                }
            }
        }
    }
}


void olivec_fill_circle(uint32_t *pixels, size_t pixels_width, size_t pixels_height, 
                        int cx, int cy, int r,
                        uint32_t color)
{
    if (r == 0) return;
    r = OLIVE_ABS(int, r);
    int x1 = cx - r;
    int x2 = cx + r;
    // if (x1 > x2) OLIVE_SWAP(int, x1, x2);
    int y1 = cy - r;
    int y2 = cy + r;
    // if (y1 > y2) OLIVE_SWAP(int, y1, y2);
    for (int y = y1; y <= y2; y++) {
        if (0 <= y && y < (int) pixels_height) {
            for (int x = x1; x <= x2; x++) {
                if (0 <= x && x < (int) pixels_width) {
                    int dx = x - cx;
                    int dy = y - cy;
                    if (dx*dx + dy*dy <= r*r) {
                        pixels[y*pixels_width + x] = color;
                    }
                }
            }
        }
    }

}

void olivec_draw_line(uint32_t *pixels, size_t pixels_width, size_t pixels_height, 
                      int x1, int y1, int x2, int y2, 
                      uint32_t color)
{
    int dx = x2 - x1;
    int dy = y2 - y1;
    if (dx != 0) {
        int c = y1 - dy*x1/dx;

        if (x1 > x2) OLIVE_SWAP(int, x1, x2);
        for (int x = x1; x <= x2; x++) {
            if (0 <= x && x < (int) pixels_width) {
                int ym = dy*x/dx + c;
                int yp = dy*(x+1)/dx + c;
                if (ym > yp) OLIVE_SWAP(int, ym, yp);
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
            if (y1 > y2) OLIVE_SWAP(int, y1, y2);
            for (int y = y1; y <= y2; y++) {
                if (0 <= y && y < (int) pixels_height) {
                    pixels[y*pixels_width + x] = color;
                }
            }
        }
    }
}

void sort_triangle_points_by_y(int *x1, int *y1, int *x2, int *y2, int *x3, int *y3)
{
    if (*y1 > *y2) {
        OLIVE_SWAP(int, *x1, *x2);
        OLIVE_SWAP(int, *y1, *y2);
    }
    if (*y2 > *y3) {
        OLIVE_SWAP(int, *x2, *x3);
        OLIVE_SWAP(int, *y2, *y3);
    }
    if (*y1 > *y2) {
        OLIVE_SWAP(int, *x1, *x2);
        OLIVE_SWAP(int, *y1, *y2);
    }
}
void olivec_fill_triangle(uint32_t *pixels, size_t width, size_t height, 
                          int x1, int y1,
                          int x2, int y2,
                          int x3, int y3,
                          uint32_t color)
{
    sort_triangle_points_by_y(&x1, &y1, &x2, &y2, &x3, &y3);
    int dx12 = x2 - x1;
    int dy12 = y2 - y1;
    int dx13 = x3 - x1;
    int dy13 = y3 - y1;
    for (int y = y1; y <= y2; y++) {
        if (0 <= y && y < (int) height) {
            int s1 = (dy12 != 0) ? (y - y1)*dx12/dy12 + x1 : x1;
            int s2 = (dy13 != 0) ? (y - y1)*dx13/dy13 + x1 : x1;
            if (s1 > s2) OLIVE_SWAP(int, s1, s2);
            for (int x = s1; x <= s2; x++) {
                if (0 <= x && x <= (int) width) {
                    pixels[y*width + x] = color;
                }
            }
        }
    }
    int dx32 = x2 - x3;
    int dy32 = y2 - y3;
    for (int y = y2; y <= y3; y++) {
        if (0 <= y && y < (int) height) {
            int s1 = (dy32 != 0) ? (y - y3)*dx32/dy32 + x3 : x3;
            int s2 = (dy13 != 0) ? (y - y3)*dx13/dy13 + x3 : x3;
            if (s1 > s2) OLIVE_SWAP(int, s1, s2);
            for (int x = s1; x <= s2; x++) {
                if (0 <= x && x <= (int) width) {
                    pixels[y*width + x] = color;
                }
            }
        }
    }
}


