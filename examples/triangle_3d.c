#define SCALE_DOWN_FACTOR 20
#include "vc.c"
#include "./assets/ppng.c"

typedef struct {
    float x, y;
} Vector2;

Vector2 make_vector2(float x, float y)
{
    Vector2 v2;
    v2.x = x;
    v2.y = y;
    return v2;
}

typedef struct {
    float x, y, z;
} Vector3;

Vector3 make_vector3(float x, float y, float z)
{
    Vector3 v3;
    v3.x = x;
    v3.y = y;
    v3.z = z;
    return v3;
}

Vector2 project_3d_2d(Vector3 v3)
{
    return make_vector2(v3.x / v3.z, v3.y / v3.z);
}

#define WIDTH  800
#define HEIGHT 600

Vector2 project_2d_scr(Vector2 v2)
{
    return make_vector2((v2.x + 1)/2*WIDTH, (1 - (v2.y + 1)/2)*HEIGHT);
}

#define BACKGROUND_COLOR 0xFF181818

uint32_t pixels[WIDTH*HEIGHT];
float    zbuffer[WIDTH*HEIGHT];
uint32_t pixels1[WIDTH*HEIGHT];
float    zbuffer1[WIDTH*HEIGHT];

float coe = 0.7;

float global_time = 1;
Olivec_Canvas render(float dt)
{
    global_time += dt;
    Olivec_Canvas oc_png = olivec_canvas(png_pixels, png_width, png_height, png_width);
    Olivec_Canvas oc = olivec_canvas(pixels, WIDTH, HEIGHT, WIDTH);
    olivec_fill(oc, BACKGROUND_COLOR);
    Olivec_Canvas zb = olivec_canvas((uint32_t*)zbuffer, WIDTH, HEIGHT, WIDTH);
    olivec_fill(zb, 0);
    float z0 = 1.5;
    {
        Vector3 v1 = make_vector3(cosf(global_time)*coe, -0.5, z0 + sinf(global_time)*coe);
        Vector3 v2 = make_vector3(cosf(global_time + PI)*coe, -0.5, z0 + sinf(global_time + PI)*coe);
        Vector3 v3 = make_vector3(0, 0.5, z0);
        Vector2 p1 = project_2d_scr(project_3d_2d(v1));
        Vector2 p2 = project_2d_scr(project_3d_2d(v2));
        Vector2 p3 = project_2d_scr(project_3d_2d(v3));
        olivec_fill_triangle3z(zb, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, 1.0f/v1.z, 1.0f/v2.z, 1.0f/v3.z);
        // olivec_fill_triangle3c(oc, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, 0xFF1818FF, 0xFF1818FF, 0xFF18FF18);
        olivec_fill_triangle3uv(oc, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y,
                                make_uv(0/v1.z, 1/v1.z),
                                make_uv(0.5/v2.z, 0/v2.z),
                                make_uv(1/v3.z, 0/v3.z),
                                1/v1.z, 1/v2.z, 1/v3.z,
                                oc_png
                                );
    }
#if 1
    Olivec_Canvas oc1 = olivec_canvas(pixels1, WIDTH, HEIGHT, WIDTH);
    olivec_fill(oc1, BACKGROUND_COLOR);
    Olivec_Canvas zb1 = olivec_canvas((uint32_t*)zbuffer1, WIDTH, HEIGHT, WIDTH);
    olivec_fill(zb1, 0);
    {
        Vector3 v1 = make_vector3(cosf(global_time + PI/2)*coe, -0.5, z0 + sinf(global_time + PI/2)*coe);
        Vector3 v2 = make_vector3(cosf(global_time - PI/2)*coe, -0.5, z0 + sinf(global_time - PI/2)*coe);
        Vector3 v3 = make_vector3(0, 0.5, z0);
        Vector2 p1 = project_2d_scr(project_3d_2d(v1));
        Vector2 p2 = project_2d_scr(project_3d_2d(v2));
        Vector2 p3 = project_2d_scr(project_3d_2d(v3));
        olivec_fill_triangle3z(zb1, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, 1.0f/v1.z, 1.0f/v2.z, 1.0f/v3.z);
        olivec_fill_triangle3c(oc1, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, 0xFFFF1818, 0xFFFF1818, 0xFF18FF18);
    }
#endif
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            float z = (*(float*)&OLIVEC_PIXEL(zb, x, y));
            float z1 = (*(float*)&OLIVEC_PIXEL(zb1, x, y));
            if (z < z1) {
                OLIVEC_PIXEL(oc, x, y) = OLIVEC_PIXEL(oc1, x, y);
                z = z1;
            }
            z = 1.0f/z;
            if (z < 1.0f) continue;
            z -= 1.0f;
            uint32_t v = z*255;
            if (v > 255) v = 255;
            olivec_blend_color(&OLIVEC_PIXEL(oc, x, y), (v<<(3*8)) | ((BACKGROUND_COLOR<<8)>>8));
        }
    }
    return oc;
}
