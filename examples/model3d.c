#include "vc.c"

#define WIDTH 960
#define HEIGHT 720
#define BACKGROUND_COLOR 0xFF181818

// #define PI 3.14159265359

// float sinf(float x);
// float cosf(float x);

static uint32_t pixels[WIDTH*HEIGHT];
static float zbuffer[WIDTH*HEIGHT] = {0};
static float angle = 0;

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
    return make_vector2(v3.x/v3.z, v3.y/v3.z);
}

Vector2 project_2d_scr(Vector2 v2)
{
    return make_vector2((v2.x + 1)/2*WIDTH, (1 - (v2.y + 1)/2)*HEIGHT);
}

float cz = 0;
Vector3 rotate_y(Vector3 p, float delta_angle)
{
    float xt = p.x - 0;
    float zt = p.z - cz;

    float cost = cosf(delta_angle);
    float sint = sinf(delta_angle);

    return make_vector3(cost*xt - sint*zt, p.y, sint*xt + cost*zt + cz);
}

Olivec_Canvas render(float dt)
{
    angle += 0.25*PI*dt;

    Olivec_Canvas oc = olivec_canvas(pixels, WIDTH, HEIGHT, WIDTH);
    olivec_fill(oc, BACKGROUND_COLOR);
    for (size_t i = 0; i < WIDTH*HEIGHT; ++i) zbuffer[i] = 0;

    for (size_t i = 0; i < faces_count; ++i) {
        int a = faces[i][0];
        int b = faces[i][1];
        int c = faces[i][2];
        Vector3 v1 = rotate_y(make_vector3(vertices[a][0], vertices[a][1], vertices[a][2]), angle);
        Vector3 v2 = rotate_y(make_vector3(vertices[b][0], vertices[b][1], vertices[b][2]), angle);
        Vector3 v3 = rotate_y(make_vector3(vertices[c][0], vertices[c][1], vertices[c][2]), angle);
        v1.z += 1.5; v2.z += 1.5; v3.z += 1.5;
        Vector2 p1 = project_2d_scr(project_3d_2d(v1));
        Vector2 p2 = project_2d_scr(project_3d_2d(v2));
        Vector2 p3 = project_2d_scr(project_3d_2d(v3));
        
        Olivec_Normalized_Rect nr = {0};
        if (!olivec_norm_rect4tri(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, oc.width, oc.height, &nr)) continue;
        int x_BC = p3.x - p2.x, y_BC = p3.y - p2.y;
        int x_CA = p1.x - p3.x, y_CA = p1.y - p3.y;
        int det = x_BC*y_CA - x_CA*y_BC; if (det == 0) continue;
        for (int x = nr.x1; x <= nr.x2; x++) {
            for (int y = nr.y1; y <= nr.y2; y++) {
                int x_CP = x - p3.x, y_CP = y - p3.y;
                int u1 = x_BC*y_CP - x_CP*y_BC;
                int u2 = x_CA*y_CP - x_CP*y_CA;
                int u3 = det - u1 - u2;
                if ((OLIVEC_SIGN(int, u1) == OLIVEC_SIGN(int, det) || u1 == 0) &&
                    (OLIVEC_SIGN(int, u2) == OLIVEC_SIGN(int, det) || u2 == 0) &&
                    (OLIVEC_SIGN(int, u3) == OLIVEC_SIGN(int, det) || u3 == 0)) {
                    float z = 1.0f/v1.z*u1/det + 1.0f/v2.z*u2/det + 1.0f/v3.z*u3/det;
                    if (z <= zbuffer[y*WIDTH + x]) continue;
                    zbuffer[y*WIDTH + x] = z;
                    // OLIVEC_PIXEL(oc, x, y) = mix_color3(0xFF1818FF, 0xFF18FF18, 0xFFFF1818, u1, u2, det);
                    olivec_blend_color(&OLIVEC_PIXEL(oc, x, y), mix_color3(0xFF1818FF, 0xFF18FF18, 0xFFFF1818, u1, u2, det));
                    z = 1.0f/z;
                    if (z < 1.0f) continue;
                    z -= 1.0f;
                    uint32_t v = z*255;
                    if (v > 255) v = 255;
                    olivec_blend_color(&OLIVEC_PIXEL(oc, x, y), (v<<(3*8)) | ((BACKGROUND_COLOR<<8)>>8));
                }
            }
        }
    }

    return oc;
}