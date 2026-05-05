#pragma once

#include <stdint.h>
#include <string.h>
#include <errno.h>

void olivec_fill(uint32_t *pixels, size_t width, size_t height, uint32_t color)
{
    for (size_t i = 0; i < width*height; i++) {
        pixels[i] = color;
    }
}

typedef int Errno;
#define return_defer(value) do { result = (value); goto defer; } while(0)
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

void olivec_fill_rect(uint32_t *pixels, size_t pixels_width, size_t pixels_height, 
                      int x0, int y0, size_t w, size_t h, 
                      uint32_t color)
{
    for (size_t dy = 0; dy < h; dy++) {
        int y = y0 + dy;
        if (0 <= y && y < (int) pixels_height) {
            for(size_t dx = 0; dx < w; dx++) {
                int x = x0 + dx;
                if (0 <= x && x < (int) pixels_width) {
                    pixels[y*pixels_width + x] = color;
                }
            }
        }
    }

}


void olivec_fill_circle(uint32_t *pixels, size_t pixels_width, size_t pixels_height, 
                        int cx, int cy, size_t r,
                        uint32_t color)
{
    int x1 = cx - (int) r;
    int y1 = cy - (int) r;
    int x2 = cx + (int) r;
    int y2 = cy + (int) r;
    for (int y = y1; y <= y2; y++) {
        if (0 <= y && y < (int) pixels_height) {
            for (int x = x1; x <= x2; x++) {
                if (0 <= x && x < (int) pixels_width) {
                    int dx = x - cx;
                    int dy = y - cy;
                    if (dx*dx + dy*dy <= (int) (r*r)) {
                        pixels[y*pixels_width + x] = color;
                    }
                }
            }
        }
    }

}

