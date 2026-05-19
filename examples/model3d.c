#include "vc.c"

#define WIDTH 960
#define HEIGHT 720
#define BACKGROUND_COLOR 0xFF181818

#define EPSILON 1e-6

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

float dot_product_vector3(Vector3 v1, Vector3 v2)
{
    return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}

Vector3 vector3_scale(Vector3 v, float t)
{
    return make_vector3(v.x*t, v.y*t, v.z*t);
}

Vector3 vector_2d_3d(Vector2 v, float z)
{
    return make_vector3(v.x, v.y, z);
}

Vector2 project_3d_2d(Vector3 v3)
{
    // if (v3.z < 0) v3.z = -v3.z;
    if (v3.z < EPSILON) v3.z = EPSILON;
    return make_vector2(v3.x/v3.z, v3.y/v3.z);
}

Vector2 project_2d_scr(Vector2 v2)
{
    return make_vector2((v2.x + 1)/2*WIDTH, (1 - (v2.y + 1)/2)*HEIGHT);
}

Vector3 rotate_y(Vector3 p, float delta_angle)
{
    float xt = p.x;
    float zt = p.z;

    float cost = cosf(delta_angle);
    float sint = sinf(delta_angle);

    return make_vector3(cost*xt - sint*zt, p.y, sint*xt + cost*zt);
}

static float near = 0.1f;
static float far = 5.0f;
// static float cx = 0.0f;
// static float cy = 0.0f;
static float cz = 1.5f;
static Vector3 camera = {0, 0 , 1};

Olivec_Canvas render(float dt)
{
    angle += 0.25*PI*dt;

    Olivec_Canvas oc = olivec_canvas(pixels, WIDTH, HEIGHT, WIDTH);
    olivec_fill(oc, BACKGROUND_COLOR);
    for (size_t i = 0; i < WIDTH*HEIGHT; ++i) zbuffer[i] = 0;

    for (size_t i = 0; i < faces_count; ++i) {
        int a, b, c;
        a = faces_v[i][0];
        b = faces_v[i][1];
        c = faces_v[i][2];
        Vector3 v1 = rotate_y(make_vector3(vertices[a][0], vertices[a][1], vertices[a][2]), angle);
        Vector3 v2 = rotate_y(make_vector3(vertices[b][0], vertices[b][1], vertices[b][2]), angle);
        Vector3 v3 = rotate_y(make_vector3(vertices[c][0], vertices[c][1], vertices[c][2]), angle);
        // v1.x += cx; v2.x += cx; v3.x += cx;
        // v1.y += cy; v2.y += cy; v3.y += cy;
        v1.z += cz; v2.z += cz; v3.z += cz;
#if 1
        Vector3 vn1 = {0}, vn2 = {0}, vn3 = {0};
        if (normals_count > 0) {
            a = faces_vn[i][0];
            b = faces_vn[i][1];
            c = faces_vn[i][2];
            vn1 = rotate_y(make_vector3(normals[a][0], normals[a][1], normals[a][2]), angle);
            vn2 = rotate_y(make_vector3(normals[b][0], normals[b][1], normals[b][2]), angle);
            vn3 = rotate_y(make_vector3(normals[c][0], normals[c][1], normals[c][2]), angle);
            // if (dot_product_vector3(camera, vn1) > 0 || dot_product_vector3(camera, vn2) > 0 || dot_product_vector3(camera, vn3) > 0) continue;
        }
#endif

#ifdef TEXTURE_RENDERING
        UV uv1 = {0}, uv2 = {0}, uv3 = {0};
        if (texcoords_count > 0) {
            a = faces_vt[i][0];
            b = faces_vt[i][1];
            c = faces_vt[i][2];
            uv1 = make_uv(texcoords[a][0], texcoords[a][1]);
            uv2 = make_uv(texcoords[b][0], texcoords[b][1]);
            uv3 = make_uv(texcoords[c][0], texcoords[c][1]);
        }
#endif
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
                if (!is_inner(u1, u2, u3, det)) continue;
#if 1
                // backface culling
                if (normals_count > 0) {
                    Vector3 vn = make_vector3(
                        (vn1.x*u1 + vn2.x*u2 + vn3.x*u3)/det,
                        (vn1.y*u1 + vn2.y*u2 + vn3.y*u3)/det,
                        (vn1.z*u1 + vn2.z*u2 + vn3.z*u3)/det);
                    if (dot_product_vector3(camera, vn) > 0) continue;
                }
#endif
                float z = (1.0f/v1.z*u1 + 1.0f/v2.z*u2 + 1.0f/v3.z*u3)/det;
                if (1.0f/far > z || z > 1.0f/near || z <= zbuffer[y*WIDTH + x]) continue;
                zbuffer[y*WIDTH + x] = z;
#ifndef TEXTURE_RENDERING
                // OLIVEC_PIXEL(oc, x, y) = mix_color3(0xFF1818FF, 0xFF18FF18, 0xFFFF1818, u1, u2, det);
                olivec_blend_color(&OLIVEC_PIXEL(oc, x, y), mix_color3(0xFF1818FF, 0xFF18FF18, 0xFFFF1818, u1, u2, det));
#else
                // texture rendering
                if (texcoords_count > 0) {
                    int texture_x = (uv1.u*u1 + uv2.u*u2 + uv3.u*u3)/det*png_width;
                    int texture_y = (uv1.v*u1 + uv2.v*u2 + uv3.v*u3)/det*png_height;
                    OLIVEC_PIXEL(oc, x, y) = png_pixels[texture_y*png_width + texture_x];
                }
#endif
#if 1
                // far/near screen rendering
                z = 1.0f/z;
                if (z < 1.0f) continue;
                z -= 1.0f;
                uint32_t v = z*255;
                if (v > 255) v = 255;
                olivec_blend_color(&OLIVEC_PIXEL(oc, x, y), (v<<(3*8)) | ((BACKGROUND_COLOR<<8)>>8));
#endif
            }
        }
    }

    return oc;
}
