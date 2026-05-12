#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef OLIVECDEF
#define OLIVECDEF static inline
#endif

#define OLIVEC_SWAP(T, a, b) do {T t = a; a = b; b = t;} while (0)
#define OLIVEC_SIGN(T, x) ((T)((x) > 0) - (T)((x) < 0))
#define OLIVEC_ABS(T, x) (OLIVEC_SIGN(T, x)*(x))

typedef struct {
    uint32_t *pixels;
    size_t width;
    size_t height;
    size_t stride;
} Olivec_Canvas;

#define OLIVEC_CANVAS_NULL ((Olivec_Canvas) {0})
#define OLIVEC_PIXEL(oc, x, y) (oc).pixels[(y)*(oc).stride + (x)]

OLIVECDEF Olivec_Canvas olivec_canvas(uint32_t *pixels, size_t width, size_t height, size_t stride);
OLIVECDEF Olivec_Canvas olivec_subcanvas(Olivec_Canvas oc, int x, int y, int w, int h);
OLIVECDEF void olivec_blend_color(uint32_t *c1, uint32_t c2);
OLIVECDEF void olivec_fill(Olivec_Canvas oc, uint32_t color);
OLIVECDEF void olivec_fill_rect(Olivec_Canvas oc, int x, int y, int w, int h, uint32_t color);
OLIVECDEF void olivec_fill_circle(Olivec_Canvas oc, int cx, int cy, int r, uint32_t color);
OLIVECDEF void olivec_fill_triangle(Olivec_Canvas oc, int x1, int y1, int x2, int y2, int x3, int y3, uint32_t color);
OLIVECDEF void olivec_fill_triangle3(Olivec_Canvas oc, int x1, int y1, int x2, int y2, int x3, int y3, uint32_t color1, uint32_t color2, uint32_t color3);
OLIVECDEF void olivec_draw_line(Olivec_Canvas oc, int x1, int y1, int x2, int y2, uint32_t color);
OLIVECDEF bool olivec_normalize_rect(int x, int y, int w, int h,
                                     size_t pixels_width, size_t pixels_height,
                                     int *x1, int *x2, int *y1, int *y2);
OLIVECDEF void olivec_copy(Olivec_Canvas dst, Olivec_Canvas src);

typedef struct {
    size_t width, height;
    const char *glyphs;
    size_t interval;
} Olivec_Font;
#include "olivec_default_font.c"
OLIVECDEF void olivec_text(Olivec_Canvas oc, const char *text, Olivec_Font font, int x, int y, size_t size, uint32_t color);

#ifdef OLIVEC_IMPLEMENTATION

OLIVECDEF Olivec_Canvas olivec_canvas(uint32_t *pixels, size_t width, size_t height, size_t stride)
{
    Olivec_Canvas oc = {
        .pixels = pixels,
        .width  = width,
        .height = height,
        .stride = stride,
    };
    return oc;
}

OLIVECDEF bool olivec_normalize_rect(int x, int y, int w, int h,
                                        size_t pixels_width, size_t pixels_height,
                                        int *x1, int *x2, int *y1, int *y2)
{
    *x1 = x;
    *y1 = y;

    // Convert the rectangle to 2-points representation
    *x2 = *x1 + OLIVEC_SIGN(int, w)*(OLIVEC_ABS(int, w) - 1);
    if (*x1 > *x2) OLIVEC_SWAP(int, *x1, *x2);
    *y2 = *y1 + OLIVEC_SIGN(int, h)*(OLIVEC_ABS(int, h) - 1);
    if (*y1 > *y2) OLIVEC_SWAP(int, *y1, *y2);

    // Cull out invisible rectangle
    if (*x1 >= (int) pixels_width) return false;
    if (*x2 < 0) return false;
    if (*y1 >= (int) pixels_height) return false;
    if (*y2 < 0) return false;

    // Clamp the rectangle to the boundaries
    if (*x1 < 0) *x1 = 0;
    if (*x2 >= (int) pixels_width) *x2 = (int) pixels_width - 1;
    if (*y1 < 0) *y1 = 0;
    if (*y2 >= (int) pixels_height) *y2 = (int) pixels_height - 1;

    return true;
}

OLIVECDEF Olivec_Canvas olivec_subcanvas(Olivec_Canvas oc, int x, int y, int w, int h)
{
    int x1, x2, y1, y2;
    if (!olivec_normalize_rect(x, y, w, h, oc.width, oc.height, &x1, &x2, &y1, &y2)) return OLIVEC_CANVAS_NULL;
    oc.pixels = &OLIVEC_PIXEL(oc, x1, y1);
    oc.width = x2 - x1 + 1;
    oc.height = y2 - y1 + 1;
    return oc;
}

#define OLIVEC_RED(color)   (((color)&0x000000FF)>>(8*0))
#define OLIVEC_GREEN(color) (((color)&0x0000FF00)>>(8*1))
#define OLIVEC_BLUE(color)  (((color)&0x00FF0000)>>(8*2))
#define OLIVEC_ALPHA(color) (((color)&0xFF000000)>>(8*3))
#define OLIVEC_RGBA(r, g, b, a) ((((r)&0xFF)<<(8*0)) | (((g)&0xFF)<<(8*1)) | (((b)&0xFF)<<(8*2)) | (((a)&0xFF)<<(8*3)))

OLIVECDEF void olivec_blend_color(uint32_t *c1, uint32_t c2)
{
    uint32_t r1 = OLIVEC_RED(*c1);
    uint32_t g1 = OLIVEC_GREEN(*c1);
    uint32_t b1 = OLIVEC_BLUE(*c1);
    uint32_t a1 = OLIVEC_ALPHA(*c1);

    uint32_t r2 = OLIVEC_RED(c2);
    uint32_t g2 = OLIVEC_GREEN(c2);
    uint32_t b2 = OLIVEC_BLUE(c2);
    uint32_t a2 = OLIVEC_ALPHA(c2);

    r1 = (r1*(255 - a2) + r2*a2)/255; if (r1 > 255) r1 = 255;
    g1 = (g1*(255 - a2) + g2*a2)/255; if (g1 > 255) g1 = 255;
    b1 = (b1*(255 - a2) + b2*a2)/255; if (b1 > 255) b1 = 255;

    *c1 = OLIVEC_RGBA(r1, g1, b1, a1);
}

OLIVECDEF void olivec_fill(Olivec_Canvas oc, uint32_t color)
{
    for (size_t y = 0; y < oc.height; ++y) {
        for (size_t x = 0; x < oc.width; ++x) {
            OLIVEC_PIXEL(oc, x, y) = color;
        }
    }
}

OLIVECDEF void olivec_fill_rect(Olivec_Canvas oc, int x, int y, int w, int h, uint32_t color)
{
    int x1, x2, y1, y2;
    if (!olivec_normalize_rect(x, y, w, h, oc.width, oc.height, &x1, &x2, &y1, &y2)) return;
    for (int x = x1; x <= x2; ++x) {
        for (int y = y1; y <= y2; ++y) {
            olivec_blend_color(&OLIVEC_PIXEL(oc, x, y), color);
        }
    }
}

OLIVECDEF void olivec_fill_circle(Olivec_Canvas oc, int cx, int cy, int r, uint32_t color)
{
    int x1, y1, x2, y2;
    int r1 = r + OLIVEC_SIGN(int, r);
    if (!olivec_normalize_rect(cx - r1, cy - r1, 2*r1, 2*r1, oc.width, oc.height, &x1, &x2, &y1, &y2)) return;
    for (int y = y1; y <= y2; y++) {
        for (int x = x1; x <= x2; x++) {
            int dx = x - cx;
            int dy = y - cy;
            if (dx*dx + dy*dy <= r*r) {
                olivec_blend_color(&OLIVEC_PIXEL(oc, x, y), color);
            }
        }
    }
}

OLIVECDEF void olivec_draw_line(Olivec_Canvas oc, int x1, int y1, int x2, int y2, uint32_t color)
{
    int dx = x2 - x1;
    int dy = y2 - y1;
    if (dx == 0 && dy == 0) {
        if (0 <= x1 && x1 < (int) oc.width && 0 <= y1 && y1 < (int) oc.height) {
            olivec_blend_color(&OLIVEC_PIXEL(oc, x1, y1), color);
        }
        return;
    }
    if (OLIVEC_ABS(int, dx) > OLIVEC_ABS(int, dy)) {
        if (x1 > x2) {
            OLIVEC_SWAP(int, x1, x2);
            OLIVEC_SWAP(int, y1, y2);
        }

        // Cull out invisible line
        if (x1 >= (int) oc.width || x2 < 0) return;
        
        // Clamp the line to the boundaries
        if (x1 < 0) x1 = 0;
        if (x2 >= (int) oc.width) x2 = (int) oc.width - 1;
        
        for (int x = x1; x <= x2; ++x) {
            int y = dy*(x - x1)/dx + y1;
            // TODO: move boundary checks out side of the loops in olivec_draw_line
            if (0 <= y && y < (int) oc.height) {
                olivec_blend_color(&OLIVEC_PIXEL(oc, x, y), color);
            }
        }
    } else {
        if (y1 > y2) {
            OLIVEC_SWAP(int, x1, x2);
            OLIVEC_SWAP(int, y1, y2);
        }

        // Cull out invisible line
        if (y1 >= (int) oc.height || y2 < 0) return;
        
        // Clamp the line to the boundaries
        if (y1 < 0) y1 = 0;
        if (y2 >= (int) oc.height) y2 = (int) oc.height - 1;

        for (int y = y1; y <= y2; ++y) {
            int x = dx*(y - y1)/dy + x1;
            // TODO: move boundary checks out side of the loops in olivec_draw_line
            if (0 <= x && x < (int) oc.width) {
                olivec_blend_color(&OLIVEC_PIXEL(oc, x, y), color);
            }
        }
    }
}

OLIVECDEF void olivec_fill_triangle(Olivec_Canvas oc, int x1, int y1, int x2, int y2, int x3, int y3, uint32_t color)
{
    if (y1 > y2) {
        OLIVEC_SWAP(int, x1, x2);
        OLIVEC_SWAP(int, y1, y2);
    }
    if (y2 > y3) {
        OLIVEC_SWAP(int, x2, x3);
        OLIVEC_SWAP(int, y2, y3);
    }
    if (y1 > y2) {
        OLIVEC_SWAP(int, x1, x2);
        OLIVEC_SWAP(int, y1, y2);
    }

    int dx12 = x2 - x1;
    int dy12 = y2 - y1;
    int dx13 = x3 - x1;
    int dy13 = y3 - y1;
    for (int y = y1; y <= y2; y++) {
        if (0 <= y && y < (int) oc.height) {
            int s1 = (dy12 != 0) ? (y - y1)*dx12/dy12 + x1 : x1;
            int s2 = (dy13 != 0) ? (y - y1)*dx13/dy13 + x1 : x1;
            if (s1 > s2) OLIVEC_SWAP(int, s1, s2);
            for (int x = s1; x <= s2; x++) {
                if (0 <= x && x <= (int) oc.width) {
                    olivec_blend_color(&OLIVEC_PIXEL(oc, x, y), color);
                }
            }
        }
    }
    int dx32 = x2 - x3;
    int dy32 = y2 - y3;
    for (int y = y2+1; y <= y3; y++) {
        if (0 <= y && y < (int) oc.height) {
            int s1 = (dy32 != 0) ? (y - y3)*dx32/dy32 + x3 : x3;
            int s2 = (dy13 != 0) ? (y - y3)*dx13/dy13 + x3 : x3;
            if (s1 > s2) OLIVEC_SWAP(int, s1, s2);
            for (int x = s1; x <= s2; x++) {
                if (0 <= x && x <= (int) oc.width) {
                    olivec_blend_color(&OLIVEC_PIXEL(oc, x, y), color);
                }
            }
        }
    }
}

uint32_t mix_color3(uint32_t c1, uint32_t c2, uint32_t c3, int u1, int u2, int det)
{
    if(det == 0) return c1;

    int64_t r1 = OLIVEC_RED  (c1);
    int64_t g1 = OLIVEC_GREEN(c1);
    int64_t b1 = OLIVEC_BLUE (c1);
    int64_t a1 = OLIVEC_ALPHA(c1);

    int64_t r2 = OLIVEC_RED  (c2);
    int64_t g2 = OLIVEC_GREEN(c2);
    int64_t b2 = OLIVEC_BLUE (c2);
    int64_t a2 = OLIVEC_ALPHA(c2);

    int64_t r3 = OLIVEC_RED  (c3);
    int64_t g3 = OLIVEC_GREEN(c3);
    int64_t b3 = OLIVEC_BLUE (c3);
    int64_t a3 = OLIVEC_ALPHA(c3);

    int u3 = det - u1 - u2;
    int64_t r = (r1*u1 + r2*u2 + r3*u3)/det;
    int64_t g = (g1*u1 + g2*u2 + g3*u3)/det;
    int64_t b = (b1*u1 + b2*u2 + b3*u3)/det;
    int64_t a = (a1*u1 + a2*u2 + a3*u3)/det;

    return OLIVEC_RGBA(r, g, b, a);
}

OLIVECDEF void olivec_fill_triangle3(Olivec_Canvas oc, int x1, int y1, int x2, int y2, int x3, int y3, uint32_t color1, uint32_t color2, uint32_t color3)
{
    if (y1 > y2) {
        OLIVEC_SWAP(int, x1, x2);
        OLIVEC_SWAP(int, y1, y2);
        OLIVEC_SWAP(uint32_t, color1, color2);
    }
    if (y2 > y3) {
        OLIVEC_SWAP(int, x2, x3);
        OLIVEC_SWAP(int, y2, y3);
        OLIVEC_SWAP(uint32_t, color2, color3);
    }
    if (y1 > y2) {
        OLIVEC_SWAP(int, x1, x2);
        OLIVEC_SWAP(int, y1, y2);
        OLIVEC_SWAP(uint32_t, color1, color2);
    }

    int det = (x1 - x3)*(y2 - y3) - (x2 - x3)*(y1 - y3);

    int dx12 = x2 - x1;
    int dy12 = y2 - y1;
    int dx13 = x3 - x1;
    int dy13 = y3 - y1;
    for (int y = y1; y <= y2; y++) {
        if (0 <= y && y < (int) oc.height) {
            int s1 = (dy12 != 0) ? (y - y1)*dx12/dy12 + x1 : x1;
            int s2 = (dy13 != 0) ? (y - y1)*dx13/dy13 + x1 : x1;
            if (s1 > s2) OLIVEC_SWAP(int, s1, s2);
            for (int x = s1; x <= s2; x++) {
                if (0 <= x && x <= (int) oc.width) {
                    int u1 = (x - x3)*(y2 - y3) - (x2 - x3)*(y - y3);
                    int u2 = (x1 - x3)*(y - y3) - (x - x3)*(y1 - y3);
                    int color = mix_color3(color1, color2, color3, u1, u2, det);
                    olivec_blend_color(&OLIVEC_PIXEL(oc, x, y), color);
                }
            }
        }
    }
    int dx32 = x2 - x3;
    int dy32 = y2 - y3;
    for (int y = y2+1; y <= y3; y++) {
        if (0 <= y && y < (int) oc.height) {
            int s1 = (dy32 != 0) ? (y - y3)*dx32/dy32 + x3 : x3;
            int s2 = (dy13 != 0) ? (y - y3)*dx13/dy13 + x3 : x3;
            if (s1 > s2) OLIVEC_SWAP(int, s1, s2);
            for (int x = s1; x <= s2; x++) {
                if (0 <= x && x <= (int) oc.width) {
                    int u1 = (x - x3)*(y2 - y3) - (x2 - x3)*(y - y3);
                    int u2 = (x1 - x3)*(y - y3) - (x - x3)*(y1 - y3);
                    int color = mix_color3(color1, color2, color3, u1, u2, det);
                    olivec_blend_color(&OLIVEC_PIXEL(oc, x, y), color);
                }
            }
        }
    }
}

OLIVECDEF void olivec_text(Olivec_Canvas oc, const char *text, Olivec_Font font, int tx, int ty, size_t glyph_size, uint32_t color){
    for (size_t i = 0; *text; i++, text++) {
        int gx = tx + i*(font.width + font.interval)*glyph_size;
        int gy = ty;
        const char *glyph = &font.glyphs[(*text)*sizeof(char)*font.height*font.width];
        for (int dy = 0; dy < (int) (font.height*glyph_size); dy++) {
            for (int dx = 0; dx < (int) (font.width*glyph_size); dx++) {
                if(!glyph[dy/glyph_size*font.width + dx/glyph_size]) continue;
                int x = gx + dx;
                int y = gy + dy;
                if (0 <= x && x < (int) oc.width && 0 <= y && y <= (int) oc.height) {
                    olivec_blend_color(&OLIVEC_PIXEL(oc, x, y), color);
                }
            }
        }
    }
}

OLIVECDEF void olivec_copy(Olivec_Canvas dst, Olivec_Canvas src)
{
    for (size_t y = 0; y < dst.height; ++y) {
        for (size_t x = 0; x < dst.width; ++x) {
            size_t nx = x*src.width/dst.width;
            size_t ny = y*src.height/dst.height;
            OLIVEC_PIXEL(dst, x, y) = OLIVEC_PIXEL(src, nx, ny);
        }
    }
}

#endif // OLIVEC_IMPLEMENTATION
