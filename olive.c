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

#define OLIVEC_RED(color)   (((color)&0x000000FF)>>(8*0))
#define OLIVEC_GREEN(color) (((color)&0x0000FF00)>>(8*1))
#define OLIVEC_BLUE(color)  (((color)&0x00FF0000)>>(8*2))
#define OLIVEC_ALPHA(color) (((color)&0xFF000000)>>(8*3))
#define OLIVEC_RGBA(r, g, b, a) ((((r)&0xFF)<<(8*0)) | (((g)&0xFF)<<(8*1)) | (((b)&0xFF)<<(8*2)) | (((a)&0xFF)<<(8*3)))

typedef struct {
    size_t width, height;
    const char *glyphs;
    size_t interval;
} Olivec_Font;
#include "olivec_default_font.c"

typedef struct {
    uint32_t *pixels;
    size_t width;
    size_t height;
    size_t stride;
} Olivec_Canvas;

typedef struct {
    // Safe ranges to iterate over.
    int x1, x2;
    int y1, y2;

    // Original uncut ranges some parts of which may be outside of the canvas boundaries.
    int ox1, ox2;
    int oy1, oy2;
} Olivec_Normalized_Rect;

#define OLIVEC_CANVAS_NULL ((Olivec_Canvas) {0})
#define OLIVEC_PIXEL(oc, x, y) (oc).pixels[(y)*(oc).stride + (x)]

typedef struct {
    float u, v;
} UV;
OLIVECDEF UV make_uv(float u, float v) 
{
    UV uv = {u, v};
    return uv;
}

OLIVECDEF Olivec_Canvas olivec_canvas(uint32_t *pixels, size_t width, size_t height, size_t stride);
OLIVECDEF Olivec_Canvas olivec_subcanvas(Olivec_Canvas oc, int x, int y, int w, int h);
OLIVECDEF void olivec_blend_color(uint32_t *c1, uint32_t c2);
OLIVECDEF void olivec_fill(Olivec_Canvas oc, uint32_t color);
OLIVECDEF void olivec_fill_rect(Olivec_Canvas oc, int x, int y, int w, int h, uint32_t color);
OLIVECDEF void olivec_frame(Olivec_Canvas oc, int x, int y, int w, int h, size_t t, uint32_t color);
OLIVECDEF void olivec_fill_circle(Olivec_Canvas oc, int cx, int cy, int r, uint32_t color);
OLIVECDEF void olivec_fill_ellipse(Olivec_Canvas oc, int cx, int cy, int rx, int ry, uint32_t color);
OLIVECDEF void olivec_fill_triangle(Olivec_Canvas oc, int x1, int y1, int x2, int y2, int x3, int y3, uint32_t color);
OLIVECDEF void olivec_fill_triangle3c(Olivec_Canvas oc, int x1, int y1, int x2, int y2, int x3, int y3, uint32_t c1, uint32_t c2, uint32_t c3);
OLIVECDEF void olivec_fill_triangle3z(Olivec_Canvas oc, int x1, int y1, int x2, int y2, int x3, int y3, float z1, float z2, float z3);
OLIVECDEF void olivec_fill_triangle3uv(Olivec_Canvas oc, int x1, int y1, int x2, int y2, int x3, int y3, UV uv1, UV uv2, UV uv3, float z1, float z2, float z3, Olivec_Canvas tex);
OLIVECDEF void olivec_fill_triangle3uv_bilinear(Olivec_Canvas oc, int x1, int y1, int x2, int y2, int x3, int y3, UV uv1, UV uv2, UV uv3, float z1, float z2, float z3, Olivec_Canvas tex);
OLIVECDEF void olivec_draw_line(Olivec_Canvas oc, int x1, int y1, int x2, int y2, uint32_t color);
OLIVECDEF void olivec_text(Olivec_Canvas oc, const char *text, Olivec_Font font, int x, int y, size_t size, uint32_t color);
OLIVECDEF void olivec_sprite_copy (Olivec_Canvas dst, Olivec_Canvas src, int x0, int y0, int w, int h);
OLIVECDEF void olivec_sprite_copy_bilinear(Olivec_Canvas dst, Olivec_Canvas src, int x0, int y0, int w, int h);
OLIVECDEF void olivec_sprite_blend(Olivec_Canvas dst, Olivec_Canvas src, int x0, int y0, int w, int h);
OLIVECDEF bool olivec_normalize_rect(int x, int y, int w, int h, size_t width, size_t height, Olivec_Normalized_Rect *nr);
OLIVECDEF bool olivec_norm_rect4tri(int x1, int y1, int x2, int y2, int x3, int y3, size_t width, size_t height, Olivec_Normalized_Rect *nr);
OLIVECDEF uint32_t bilinear_pixel(Olivec_Canvas src, int nx, int ny, int w, int h);


#define PI 3.14159265359
float sinf(float x);
float cosf(float x);
OLIVECDEF void rotate_point(float *x, float *y, float cx, float cy, float beta);

#ifdef OLIVEC_IMPLEMENTATION

// TODO: Specify the origin, axis, and angle to achieve spatial rotation
OLIVECDEF void rotate_point(float *x, float *y, float cx, float cy, float theta)
{
    float xt = *x - cx;
    float yt = *y - cy;

    *x = cosf(theta)*xt - sinf(theta)*yt + cx;
    *y = sinf(theta)*xt + cosf(theta)*yt + cy;
}

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

OLIVECDEF bool olivec_normalize_rect(int x, int y, int w, int h, size_t width, size_t height, Olivec_Normalized_Rect *nr)
{
    // No need to render empty rectangle
    if (w == 0 || h == 0) return false;

    nr->ox1 = x;
    nr->oy1 = y;

    // Convert the rectangle to 2-points representation
    nr->ox2 = nr->ox1 + OLIVEC_SIGN(int, w)*(OLIVEC_ABS(int, w) - 1);
    if (nr->ox1 > nr->ox2) OLIVEC_SWAP(int, nr->ox1, nr->ox2);
    nr->oy2 = nr->oy1 + OLIVEC_SIGN(int, h)*(OLIVEC_ABS(int, h) - 1);
    if (nr->oy1 > nr->oy2) OLIVEC_SWAP(int, nr->oy1, nr->oy2);

    // Cull out invisible rectangle
    if (nr->ox1 >= (int) width  || nr->ox2 < 0 || nr->oy1 >= (int) height || nr->oy2 < 0) return false;

    nr->x1 = nr->ox1;
    nr->y1 = nr->oy1;
    nr->x2 = nr->ox2;
    nr->y2 = nr->oy2;

    // Clamp the rectangle to the boundaries
    if (nr->x1 < 0) nr->x1 = 0;
    if (nr->x2 >= (int) width) nr->x2 = (int) width - 1;
    if (nr->y1 < 0) nr->y1 = 0;
    if (nr->y2 >= (int) height) nr->y2 = (int) height - 1;

    return true;
}

OLIVECDEF Olivec_Canvas olivec_subcanvas(Olivec_Canvas oc, int x, int y, int w, int h)
{
    Olivec_Normalized_Rect nr = {0};
    if (!olivec_normalize_rect(x, y, w, h, oc.width, oc.height, &nr)) return OLIVEC_CANVAS_NULL;
    oc.pixels = &OLIVEC_PIXEL(oc, nr.x1, nr.y1);
    oc.width  = nr.x2 - nr.x1 + 1;
    oc.height = nr.y2 - nr.y1 + 1;
    return oc;
}

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

    r1 = (r1*(255 - a2) + r2*a2)/255;
    g1 = (g1*(255 - a2) + g2*a2)/255;
    b1 = (b1*(255 - a2) + b2*a2)/255;

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
    Olivec_Normalized_Rect nr = {0};
    if (!olivec_normalize_rect(x, y, w, h, oc.width, oc.height, &nr)) return;
    for (int x = nr.x1; x <= nr.x2; ++x) {
        for (int y = nr.y1; y <= nr.y2; ++y) {
            olivec_blend_color(&OLIVEC_PIXEL(oc, x, y), color);
        }
    }
}

OLIVECDEF void olivec_frame(Olivec_Canvas oc, int x, int y, int w, int h, size_t t, uint32_t color)
{
    if (t == 0) return; // Nothing to render

    // Convert the rectangle to 2-points representation
    int x1 = x;
    int y1 = y;
    int x2 = x1 + OLIVEC_SIGN(int, w)*(OLIVEC_ABS(int, w) - 1);
    if (x1 > x2) OLIVEC_SWAP(int, x1, x2);
    int y2 = y1 + OLIVEC_SIGN(int, h)*(OLIVEC_ABS(int, h) - 1);
    if (y1 > y2) OLIVEC_SWAP(int, y1, y2);

    olivec_fill_rect(oc, x1 - t/2, y1 - t/2, (x2 - x1 + 1) + t/2*2, t, color);  // Top
    olivec_fill_rect(oc, x1 - t/2, y1 - t/2, t, (y2 - y1 + 1) + t/2*2, color);  // Left
    olivec_fill_rect(oc, x1 - t/2, y2 + t/2, (x2 - x1 + 1) + t/2*2, -t, color); // Bottom
    olivec_fill_rect(oc, x2 + t/2, y1 - t/2, -t, (y2 - y1 + 1) + t/2*2, color); // Right
}

OLIVECDEF void olivec_fill_circle(Olivec_Canvas oc, int cx, int cy, int r, uint32_t color)
{
    Olivec_Normalized_Rect nr = {0};
    int r1 = r + OLIVEC_SIGN(int, r);
    if (!olivec_normalize_rect(cx - r1, cy - r1, 2*r1, 2*r1, oc.width, oc.height, &nr)) return;
    for (int y = nr.y1; y <= nr.y2; ++y) {
        for (int x = nr.x1; x <= nr.x2; ++x) {
            int dx = x - cx;
            int dy = y - cy;
            if (dx*dx + dy*dy <= r*r) {
                olivec_blend_color(&OLIVEC_PIXEL(oc, x, y), color);
            }
        }
    }
}

OLIVECDEF void olivec_fill_ellipse(Olivec_Canvas oc, int cx, int cy, int rx, int ry, uint32_t color)
{
    Olivec_Normalized_Rect nr = {0};
    int r1 = rx + OLIVEC_SIGN(int, rx);
    int r2 = ry + OLIVEC_SIGN(int, ry);
    if (!olivec_normalize_rect(cx - r1, cy - r2, 2*r1, 2*r2, oc.width, oc.height, &nr)) return;
    for (int y = nr.y1; y <= nr.y2; ++y) {
        for (int x = nr.x1; x <= nr.x2; ++x) {
            int dx = (x - cx)*ry;
            int dy = (y - cy)*rx;
            if (dx*dx + dy*dy <= rx*rx*ry*ry) {
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
        
        for (int x = x1; x <= x2; ++x) {
            if (0 > x || x >= (int) oc.width) continue;
            int y = dy*(x - x1)/dx + y1;
            if (0 > y || y >= (int) oc.height) continue;
            olivec_blend_color(&OLIVEC_PIXEL(oc, x, y), color);
        }
    } else {
        if (y1 > y2) {
            OLIVEC_SWAP(int, x1, x2);
            OLIVEC_SWAP(int, y1, y2);
        }

        for (int y = y1; y <= y2; ++y) {
            if (0 > y || y >= (int) oc.height) continue;
            int x = dx*(y - y1)/dy + x1;
            if (0 > x || x >= (int) oc.width) continue;
            olivec_blend_color(&OLIVEC_PIXEL(oc, x, y), color);
        }
    }
}

#define OLIVEC_MAX(T, a, b) ((T)(a) > (T)(b) ? (T)(a) : (T)(b))
#define OLIVEC_MIN(T, a, b) ((T)(a) < (T)(b) ? (T)(a) : (T)(b))
OLIVECDEF bool olivec_norm_rect4tri(int x1, int y1, int x2, int y2, int x3, int y3, size_t width, size_t height, Olivec_Normalized_Rect *nr)
{
    int xmin = OLIVEC_MIN(int, x1, OLIVEC_MIN(int, x2, x3)); if (xmin >= (int) width) return false;
    int xmax = OLIVEC_MAX(int, x1, OLIVEC_MAX(int, x2, x3)); if (xmax < 0) return false;
    int ymin = OLIVEC_MIN(int, y1, OLIVEC_MIN(int, y2, y3)); if (ymin >= (int) height) return false;
    int ymax = OLIVEC_MAX(int, y1, OLIVEC_MAX(int, y2, y3)); if (ymax < 0) return false;

    if (xmin < 0) xmin = 0;
    if (xmax >= (int) width) xmax = width - 1;
    if (ymin < 0) ymin = 0;
    if (ymax >= (int) height) ymax = height - 1;

    nr->x1 = xmin;
    nr->y1 = ymin;
    nr->x2 = xmax;
    nr->y2 = ymax;

    return true;
}

OLIVECDEF bool is_inner(int u1, int u2, int u3, int det)
{
    return (OLIVEC_SIGN(int, u1) == OLIVEC_SIGN(int, det) || u1 == 0) &&
           (OLIVEC_SIGN(int, u2) == OLIVEC_SIGN(int, det) || u2 == 0) &&
           (OLIVEC_SIGN(int, u3) == OLIVEC_SIGN(int, det) || u3 == 0);
}

OLIVECDEF void olivec_fill_triangle(Olivec_Canvas oc, int x1, int y1, int x2, int y2, int x3, int y3, uint32_t color)
{
    Olivec_Normalized_Rect nr = {0};
    if (!olivec_norm_rect4tri(x1, y1, x2, y2, x3, y3, oc.width, oc.height, &nr)) return;

    int x_BC = x3 - x2, y_BC = y3 - y2;
    int x_CA = x1 - x3, y_CA = y1 - y3;
    int det = x_BC*y_CA - x_CA*y_BC; if (det == 0) return;
    for (int x = nr.x1; x <= nr.x2; x++) {
        for (int y = nr.y1; y <= nr.y2; y++) {
            int x_CP = x - x3, y_CP = y - y3;
            int u1 = x_BC*y_CP - x_CP*y_BC;
            int u2 = x_CA*y_CP - x_CP*y_CA;
            int u3 = det - u1 - u2;
            if (is_inner(u1, u2, u3, det)) {
                 olivec_blend_color(&OLIVEC_PIXEL(oc, x, y), color);
            }
        }
    }
}

uint32_t mix_color3(uint32_t c1, uint32_t c2, uint32_t c3, int u1, int u2, int det)
{
    if(det == 0) return c1;

    int r1 = OLIVEC_RED  (c1);
    int g1 = OLIVEC_GREEN(c1);
    int b1 = OLIVEC_BLUE (c1);
    int a1 = OLIVEC_ALPHA(c1);

    int r2 = OLIVEC_RED  (c2);
    int g2 = OLIVEC_GREEN(c2);
    int b2 = OLIVEC_BLUE (c2);
    int a2 = OLIVEC_ALPHA(c2);

    int r3 = OLIVEC_RED  (c3);
    int g3 = OLIVEC_GREEN(c3);
    int b3 = OLIVEC_BLUE (c3);
    int a3 = OLIVEC_ALPHA(c3);

    int u3 = det - u1 - u2;
    int r = (r1*u1 + r2*u2 + r3*u3)/det;
    int g = (g1*u1 + g2*u2 + g3*u3)/det;
    int b = (b1*u1 + b2*u2 + b3*u3)/det;
    int a = (a1*u1 + a2*u2 + a3*u3)/det;

    return OLIVEC_RGBA(r, g, b, a);
}

OLIVECDEF void olivec_fill_triangle3c(Olivec_Canvas oc, int x1, int y1, int x2, int y2, int x3, int y3, uint32_t c1, uint32_t c2, uint32_t c3)
{
    Olivec_Normalized_Rect nr = {0};
    if (!olivec_norm_rect4tri(x1, y1, x2, y2, x3, y3, oc.width, oc.height, &nr)) return;

    int x_BC = x3 - x2, y_BC = y3 - y2;
    int x_CA = x1 - x3, y_CA = y1 - y3;
    int det = x_BC*y_CA - x_CA*y_BC; if (det == 0) return;
    for (int x = nr.x1; x <= nr.x2; x++) {
        for (int y = nr.y1; y <= nr.y2; y++) {
            int x_CP = x - x3, y_CP = y - y3;
            int u1 = x_BC*y_CP - x_CP*y_BC;
            int u2 = x_CA*y_CP - x_CP*y_CA;
            int u3 = det - u1 - u2;
            if (!is_inner(u1, u2, u3, det)) continue;
            olivec_blend_color(&OLIVEC_PIXEL(oc, x, y), mix_color3(c1, c2, c3, u1, u2, det));
        }
    }
}

OLIVECDEF void olivec_fill_triangle3z(Olivec_Canvas oc, int x1, int y1, int x2, int y2, int x3, int y3, float z1, float z2, float z3)
{
    Olivec_Normalized_Rect nr = {0};
    if (!olivec_norm_rect4tri(x1, y1, x2, y2, x3, y3, oc.width, oc.height, &nr)) return;

    int x_BC = x3 - x2, y_BC = y3 - y2;
    int x_CA = x1 - x3, y_CA = y1 - y3;
    int det = x_BC*y_CA - x_CA*y_BC; if (det == 0) return;
    for (int x = nr.x1; x <= nr.x2; x++) {
        for (int y = nr.y1; y <= nr.y2; y++) {
            int x_CP = x - x3, y_CP = y - y3;
            int u1 = x_BC*y_CP - x_CP*y_BC;
            int u2 = x_CA*y_CP - x_CP*y_CA;
            int u3 = det - u1 - u2;
            if (!is_inner(u1, u2, u3, det)) continue;
            float z = z1*u1/det + z2*u2/det + z3*u3/det;
            *(float*)&OLIVEC_PIXEL(oc, x, y) = z;
        }
    }
}

OLIVECDEF void olivec_fill_triangle3uv(Olivec_Canvas oc, int x1, int y1, int x2, int y2, int x3, int y3, UV uv1, UV uv2, UV uv3, float z1, float z2, float z3, Olivec_Canvas tex)
{
    Olivec_Normalized_Rect nr = {0};
    if (!olivec_norm_rect4tri(x1, y1, x2, y2, x3, y3, oc.width, oc.height, &nr)) return;

    int x_BC = x3 - x2, y_BC = y3 - y2;
    int x_CA = x1 - x3, y_CA = y1 - y3;
    int det = x_BC*y_CA - x_CA*y_BC; if (det == 0) return;
    for (int x = nr.x1; x <= nr.x2; x++) {
        for (int y = nr.y1; y <= nr.y2; y++) {
            int x_CP = x - x3, y_CP = y - y3;
            int u1 = x_BC*y_CP - x_CP*y_BC;
            int u2 = x_CA*y_CP - x_CP*y_CA;
            int u3 = det - u1 - u2;
            if (!is_inner(u1, u2, u3, det)) continue;
            float z = z1*u1 + z2*u2 + z3*u3;
            int texture_x = (uv1.u*u1 + uv2.u*u2 + uv3.u*u3)*tex.width/z;
            if (texture_x < 0) texture_x = 0;
            if ((size_t) texture_x >= tex.width) texture_x = tex.width - 1;
            int texture_y = (uv1.v*u1 + uv2.v*u2 + uv3.v*u3)*tex.height/z;
            if (texture_y < 0) texture_y = 0;
            if ((size_t) texture_y >= tex.height) texture_y = tex.height - 1;
            OLIVEC_PIXEL(oc, x, y) = OLIVEC_PIXEL(tex, texture_x, texture_y);
        }
    }
}
OLIVECDEF void olivec_fill_triangle3uv_bilinear(Olivec_Canvas oc, int x1, int y1, int x2, int y2, int x3, int y3, UV uv1, UV uv2, UV uv3, float z1, float z2, float z3, Olivec_Canvas tex)
{
    Olivec_Normalized_Rect nr = {0};
    if (!olivec_norm_rect4tri(x1, y1, x2, y2, x3, y3, oc.width, oc.height, &nr)) return;

    int x_BC = x3 - x2, y_BC = y3 - y2;
    int x_CA = x1 - x3, y_CA = y1 - y3;
    int det = x_BC*y_CA - x_CA*y_BC; if (det == 0) return;
    for (int x = nr.x1; x <= nr.x2; x++) {
        for (int y = nr.y1; y <= nr.y2; y++) {
            int x_CP = x - x3, y_CP = y - y3;
            int u1 = x_BC*y_CP - x_CP*y_BC;
            int u2 = x_CA*y_CP - x_CP*y_CA;
            int u3 = det - u1 - u2;
            if ((OLIVEC_SIGN(int, u1) == OLIVEC_SIGN(int, det) || u1 == 0) &&
                (OLIVEC_SIGN(int, u2) == OLIVEC_SIGN(int, det) || u2 == 0) &&
                (OLIVEC_SIGN(int, u3) == OLIVEC_SIGN(int, det) || u3 == 0)) {
                float z = z1*u1 + z2*u2 + z3*u3;
                float texture_x = (uv1.u*u1 + uv2.u*u2 + uv3.u*u3)*tex.width/z;
                if (texture_x < 0) texture_x = 0;
                if (texture_x >= (float) tex.width) texture_x = tex.width - 1;
                float texture_y = (uv1.v*u1 + uv2.v*u2 + uv3.v*u3)*tex.height/z;
                if (texture_y < 0) texture_y = 0;
                if (texture_y >= (float) tex.height) texture_y = tex.height - 1;

                int precision = 100;
                OLIVEC_PIXEL(oc, x, y) = bilinear_pixel(tex, texture_x*precision, texture_y*precision, precision, precision);
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

OLIVECDEF void olivec_sprite_blend(Olivec_Canvas dst, Olivec_Canvas src, int x0, int y0, int w, int h)
{
    if (src.width == 0 || src.height == 0) return;

    Olivec_Normalized_Rect nr = {0};
    if (!olivec_normalize_rect(x0, y0, w, h, dst.width, dst.height, &nr)) return;

    int xa = nr.ox1; if (w < 0) xa = nr.ox2;
    int ya = nr.oy1; if (h < 0) ya = nr.oy2;
    for (int y = nr.y1; y <= nr.y2; ++y) {
        for (int x = nr.x1; x <= nr.x2; ++x) {
            size_t nx = (x - xa)*((int) src.width)/w;
            size_t ny = (y - ya)*((int) src.height)/h;
            olivec_blend_color(&OLIVEC_PIXEL(dst, x, y), OLIVEC_PIXEL(src, nx, ny));
        }
    }
}

OLIVECDEF void olivec_sprite_copy(Olivec_Canvas dst, Olivec_Canvas src, int x0, int y0, int w, int h)
{
    if (src.width == 0 || src.height == 0) return;

    Olivec_Normalized_Rect nr = {0};
    if (!olivec_normalize_rect(x0, y0, w, h, dst.width, dst.height, &nr)) return;

    int xa = nr.ox1; if (w < 0) xa = nr.ox2;
    int ya = nr.oy1; if (h < 0) ya = nr.oy2;
    for (int y = nr.y1; y <= nr.y2; ++y) {
        for (int x = nr.x1; x <= nr.x2; ++x) {
            size_t nx = (x - xa)*((int) src.width)/w;
            size_t ny = (y - ya)*((int) src.height)/h;
            OLIVEC_PIXEL(dst, x, y) = OLIVEC_PIXEL(src, nx, ny);
        }
    }
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

OLIVECDEF uint32_t bilinear_pixel(Olivec_Canvas src, int nx, int ny, int w, int h)
{
    int nx1 = nx + w;
    int ny1 = ny + h;
    if (nx1/w >= (int) src.width) nx1 = (src.width - 1)*w;
    if (ny1/h >= (int) src.height) ny1 = (src.height - 1)*h;
    uint32_t c1 = OLIVEC_PIXEL(src, nx/w, ny/h);
    uint32_t c2 = OLIVEC_PIXEL(src, nx1/w, ny/h);
    uint32_t c3 = OLIVEC_PIXEL(src, nx/w, ny1/h);
    uint32_t c4 = OLIVEC_PIXEL(src, nx1/w, ny1/h);
    uint32_t c12 = mix_color2(nx, c1, nx/w*w, c2, nx1);
    uint32_t c34 = mix_color2(nx, c3, nx/w*w, c4, nx1);
    return mix_color2(ny, c12, ny/h*h, c34, ny1);
}

OLIVECDEF void olivec_sprite_copy_bilinear(Olivec_Canvas dst, Olivec_Canvas src, int x0, int y0, int w, int h)
{
    if (src.width == 0 || src.height == 0) return;

    Olivec_Normalized_Rect nr = {0};
    if (!olivec_normalize_rect(x0, y0, w, h, dst.width, dst.height, &nr)) return;

    int xa = nr.ox1; if (w < 0) xa = nr.ox2;
    int ya = nr.oy1; if (h < 0) ya = nr.oy2;
    for (int y = nr.y1; y <= nr.y2; ++y) {
        for (int x = nr.x1; x <= nr.x2; ++x) {
            int nx  = (x - xa)*((int) src.width);
            int ny  = (y - ya)*((int) src.height);
            OLIVEC_PIXEL(dst, x, y) = bilinear_pixel(src, nx, ny, w, h);
        }
    }
}

#endif // OLIVEC_IMPLEMENTATION
