#include <stdio.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <string.h>

#define SV_IMPLEMENTATION
#include "sv.h"

#define ARENA_IMPLEMENTATION
#include "arena.h"

#define return_defer(value) do { result = (value); goto defer; } while (0)
typedef int Errno;
#define UNUSED(x) (void)(x)

const char *shift(int *argc, char ***argv)
{
    assert(*argc > 0);
    const char *result = *argv[0];
    *argc -= 1;
    *argv += 1;
    return result;
}

static Arena default_arena = {0};
static Arena *context_arena = &default_arena;

static void *context_alloc(size_t size)
{
    assert(context_arena);
    return arena_alloc(context_arena, size);
}

static void *context_realloc(void *oldp, size_t oldsz, size_t newsz)
{
    if (newsz <= oldsz) return oldp;
    return memcpy(context_alloc(newsz), oldp, oldsz);
}

Errno read_entire_file(const char *file_path, char **buffer, size_t *buffer_size)
{
    Errno result = 0;
    FILE *f = NULL;

    f = fopen(file_path, "rb");
    if (f == NULL) return_defer(errno);

    if (fseek(f, 0, SEEK_END) < 0) return_defer(errno);
    long m = ftell(f);
    if (m < 0) return_defer(errno);
    if (fseek(f, 0, SEEK_SET) < 0) return_defer(errno);

    *buffer_size = m;
    *buffer = context_alloc(*buffer_size);

    fread(*buffer, *buffer_size, 1, f);
    if (ferror(f)) return_defer(errno);

defer:
    if (f) fclose(f);
    return result;
}

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

typedef struct {
    Vector3 *items;
    size_t capacity;
    size_t count;
} Vertices;

typedef struct {
    Vector2 *items;
    size_t capacity;
    size_t count;
} TexCoords;

#define VERTICES_PER_FACE 3
typedef struct {
    int v[VERTICES_PER_FACE];
    int vt[VERTICES_PER_FACE];
    int vn[VERTICES_PER_FACE];
} Face;

Face make_face(int v1, int v2, int v3, int vt1, int vt2, int vt3, int vn1, int vn2, int vn3)
{
    Face f = {
        .v  = {v1, v2, v3},
        .vt = {vt1, vt2, vt3},
        .vn = {vn1, vn2, vn3},
    };
    return f;
}

typedef struct {
    Face *items;
    size_t capacity;
    size_t count;
} Faces;

#define DA_INIT_CAPACITY 8192
#define DA_REALLOC context_realloc
#define da_append(da, item)                                                 \
    do {                                                                    \
        if ((da)->count >= (da)->capacity) {                                \
            size_t new_capacity = (da)->capacity*2;                         \
            if (new_capacity == 0) {                                        \
                new_capacity = DA_INIT_CAPACITY;                            \
            }                                                               \
                                                                            \
            (da)->items = DA_REALLOC((da)->items,                           \
                                     (da)->capacity*sizeof((da)->items[0]), \
                                     new_capacity*sizeof((da)->items[0]));  \
            (da)->capacity = new_capacity;                                  \
        }                                                                   \
                                                                            \
        (da)->items[(da)->count++] = (item);                                \
    } while (0)

void generate_code(FILE *out, Vertices vertices, Vertices normals, TexCoords texcoords, Faces faces)
{
    fprintf(out, "#ifndef OBJ_H_\n");
    fprintf(out, "#define OBJ_H_\n");

    fprintf(out, "#define vertices_count %zu\n", vertices.count);
    if (vertices.count == 0) {
        fprintf(out, "static const float vertices[1][3] = {0};\n");
    } else {
        fprintf(out, "static const float vertices[][3] = {\n");
        for (size_t i = 0; i < vertices.count; ++i) {
            Vector3 v = vertices.items[i];
            fprintf(out, "    {%f, %f, %f},\n", v.x, v.y, v.z);
        }
        fprintf(out, "};\n");
    }

    fprintf(out, "#define normals_count %zu\n", normals.count);
    if (normals.count == 0) {
        fprintf(out, "static const float normals[1][3] = {0};\n");
    } else {
        fprintf(out, "static const float normals[][3] = {\n");
        for (size_t i = 0; i < normals.count; ++i) {
            Vector3 vn = normals.items[i];
            fprintf(out, "    {%f, %f, %f},\n", vn.x, vn.y, vn.z);
        }
        fprintf(out, "};\n");
    }

    fprintf(out, "#define texcoords_count %zu\n", texcoords.count);
    if (texcoords.count == 0) {
        fprintf(out, "static const float texcoords[1][2] = {0};\n");
    } else {
        fprintf(out, "static const float texcoords[][2] = {\n");
        for (size_t i = 0; i < texcoords.count; ++i) {
            Vector2 vt = texcoords.items[i];
            fprintf(out, "    {%f, %f},\n", vt.x, vt.y);
        }
        fprintf(out, "};\n");
    }

    fprintf(out, "#define faces_count %zu\n", faces.count);
    if (faces.count == 0) {
        fprintf(out, "static const int faces_v[1][3] = {0};\n");
        fprintf(out, "static const int faces_vt[1][3] = {0};\n");
        fprintf(out, "static const int faces_vn[1][3] = {0};\n");
    } else {
        fprintf(out, "static const int faces_v[%zu][3] = {\n", faces.count);
        for (size_t i = 0; i < faces.count; ++i) {
            Face f = faces.items[i];
            fprintf(out, "    {%d, %d, %d},\n", f.v[0], f.v[1], f.v[2]);
        }
        fprintf(out, "};\n");

        if (texcoords.count == 0) {
            fprintf(out, "static const int faces_vt[1][3] = {0};\n");
        } else {
            fprintf(out, "static const int faces_vt[%zu][3] = {\n", faces.count);
            for (size_t i = 0; i < faces.count; ++i) {
                Face f = faces.items[i];
                fprintf(out, "    {%d, %d, %d},\n", f.vt[0], f.vt[1], f.vt[2]);
            }
            fprintf(out, "};\n");
        }

        if (normals.count == 0) {
            fprintf(out, "static const int faces_vn[1][3] = {0};\n");
        } else {
            fprintf(out, "static const int faces_vn[%zu][3] = {\n", faces.count);
            for (size_t i = 0; i < faces.count; ++i) {
                Face f = faces.items[i];
                fprintf(out, "    {%d, %d, %d},\n", f.vn[0], f.vn[1], f.vn[2]);
            }
            fprintf(out, "};\n");
        }
    }

    fprintf(out, "#endif // OBJ_H_\n");
}

Vector3 remap_object(Vector3 v, float scale, float lx, float hx, float ly, float hy, float lz, float hz)
{
    float cx = lx + (hx - lx)/2;
    float cy = ly + (hy - ly)/2;
    float cz = lz + (hz - lz)/2;
    v.z = (v.z - cz)*scale;
    v.x = (v.x - cx)*scale;
    v.y = (v.y - cy)*scale;
    return v;
}

void usage(const char *program_name)
{
    fprintf(stderr, "Usage: %s [OPTIONS] <INPUT.obj>\n", program_name);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "    -o    output file path\n");
    fprintf(stderr, "    -s    scale the model\n");
}

void parse_face_triple(String_View *line, int *lf, int *hf, int *v, int *vt, int *vn)
{
    char *endptr;

    *line = sv_trim_left(*line);
    *v = strtol(line->data, &endptr, 10) - 1; // NOTE: -1 is to account for 1-based indexing.
    if (*lf > *v) *lf = *v;
    if (*hf < *v) *hf = *v;
    sv_chop_left(line, endptr - line->data);
    *vt = 0;
    if (line->count > 0 && line->data[0] == '/') {
        sv_chop_left(line, 1);
        *vt = strtol(line->data, &endptr, 10) - 1; // NOTE: -1 is to account for 1-based indexing.
        sv_chop_left(line, endptr - line->data);
    }
    *vn = 0;
    if (line->count > 0 && line->data[0] == '/') {
        sv_chop_left(line, 1);
        *vn = strtol(line->data, &endptr, 10) - 1; // NOTE: -1 is to account for 1-based indexing.
        sv_chop_left(line, endptr - line->data);
    }
    while (line->count > 0 && !isspace(*line->data)) sv_chop_left(line, 1);
}

int main(int argc, char **argv)
{
    int result = 0;

    assert(argc > 0);
    const char *program_name = shift(&argc, &argv);
    const char *output_file_path = NULL;
    const char *input_file_path = NULL;
    float scale = 0.75;

    // TODO: consider using https://github.com/tsoding/flag.h in here
    while (argc > 0) {
        const char *flag = shift(&argc, &argv);
        if (strcmp(flag, "-o") == 0) {
            if (argc <= 0) {
                usage(program_name);
                fprintf(stderr, "ERROR: no value is provided for flag %s\n", flag);
                return_defer(1);
            }

            if (output_file_path != NULL) {
                usage(program_name);
                fprintf(stderr, "ERROR: %s was already provided\n", flag);
                return_defer(1);
            }

            output_file_path = shift(&argc, &argv);
        } else if (strcmp(flag, "-s") == 0) {
            if (argc <= 0) {
                usage(program_name);
                fprintf(stderr, "ERROR: no value is provided for flag %s\n", flag);
                return_defer(1);
            }

            const char *value = shift(&argc, &argv);
            scale = strtof(value, NULL);
        } else {
            if (input_file_path != NULL) {
                usage(program_name);
                fprintf(stderr, "ERROR: input file path was already provided\n");
                return_defer(1);
            }
            input_file_path = flag;
        }
    }

    if (input_file_path == NULL) {
        usage(program_name);
        fprintf(stderr, "ERROR: no input file path is provided\n");
        return_defer(1);
    }

    if (output_file_path == NULL) {
        usage(program_name);
        fprintf(stderr, "ERROR: no output file path is provided\n");
        return_defer(1);
    }

    char *buffer;
    size_t buffer_size;
    Errno err = read_entire_file(input_file_path, &buffer, &buffer_size);
    if (err != 0) {
        fprintf(stderr, "ERROR: could not read file %s: %s\n", input_file_path, strerror(errno));
        return_defer(1);
    }

    String_View content = sv_from_parts(buffer, buffer_size);
    Vertices vertices = {0};
    Vertices vnormals = {0};
    TexCoords tcoords = {0};
    Faces faces = {0};
    float lx = FLT_MAX, hx = FLT_MIN;
    float ly = FLT_MAX, hy = FLT_MIN;
    float lz = FLT_MAX, hz = FLT_MIN;
    float ltx = FLT_MAX, htx = FLT_MIN;
    float lty = FLT_MAX, hty = FLT_MIN;
    int lf = INT_MAX, hf = INT_MIN;
    bool one_object_encountered = false;
    size_t one_object_line_number = 0;
    for (size_t line_number = 0; content.count > 0; ++line_number) {
        String_View line = sv_trim_left(sv_chop_by_delim(&content, '\n'));
        if (line.count > 0 && *line.data != '#') {
            String_View kind = sv_chop_by_delim(&line, ' ');
            if (sv_eq(kind, SV("v"))) {
                char *endptr;

                line = sv_trim_left(line);
                float x = strtof(line.data, &endptr);
                if (lx > x) lx = x;
                if (hx < x) hx = x;
                sv_chop_left(&line, endptr - line.data);

                line = sv_trim_left(line);
                float y = strtof(line.data, &endptr);
                if (ly > y) ly = y;
                if (hy < y) hy = y;
                sv_chop_left(&line, endptr - line.data);

                line = sv_trim_left(line);
                float z = strtof(line.data, &endptr);
                if (lz > z) lz = z;
                if (hz < z) hz = z;
                sv_chop_left(&line, endptr - line.data);

                da_append(&vertices, make_vector3(x, y, z));
            } else if (sv_eq(kind, SV("vn"))) {
                char *endptr;

                line = sv_trim_left(line);
                float x = strtof(line.data, &endptr);
                sv_chop_left(&line, endptr - line.data);

                line = sv_trim_left(line);
                float y = strtof(line.data, &endptr);
                sv_chop_left(&line, endptr - line.data);

                line = sv_trim_left(line);
                float z = strtof(line.data, &endptr);
                sv_chop_left(&line, endptr - line.data);

                da_append(&vnormals, make_vector3(x, y, z));
            } else if (sv_eq(kind, SV("vt"))) {
                char *endptr;

                line = sv_trim_left(line);
                float x = strtof(line.data, &endptr);
                if (ltx > x) ltx = x;
                if (htx < x) htx = x;
                sv_chop_left(&line, endptr - line.data);

                line = sv_trim_left(line);
                float y = strtof(line.data, &endptr);
                if (lty > y) lty = y;
                if (hty < y) hty = y;
                sv_chop_left(&line, endptr - line.data);

                da_append(&tcoords, make_vector2(x, y));
            } else if (sv_eq(kind, SV("f"))) {

                int v1, v2, v3, vt1, vt2, vt3, vn1, vn2, vn3;

                parse_face_triple(&line, &lf, &hf, &v1, &vt1, &vn1);

                parse_face_triple(&line, &lf, &hf, &v2, &vt2, &vn2);

                parse_face_triple(&line, &lf, &hf, &v3, &vt3, &vn3);

                da_append(&faces, make_face(v1, v2, v3, vt1, vt2, vt3, vn1, vn2, vn3));

            } else if (sv_eq(kind, sv_from_cstr("mtllib"))) {
                fprintf(stderr, "%s:%zu: WARNING: mtllib is not supported yet. Ignoring it...\n", input_file_path, line_number);
            } else if (sv_eq(kind, sv_from_cstr("usemtl"))) {
                fprintf(stderr, "%s:%zu: WARNING: usemtl is not supported yet. Ignoring it...\n", input_file_path, line_number);
            } else if (sv_eq(kind, sv_from_cstr("o"))) {
                if (one_object_encountered) {
                    fprintf(stderr, "%s:%zu: ERROR: %s supports only one object as of right now.\n", input_file_path, line_number, program_name);
                    fprintf(stderr, "%s:%zu: NOTE: we already processing this object\n", input_file_path, one_object_line_number);
                    return_defer(1);
                }
                line = sv_trim_left(line);
                String_View name = line;
                fprintf(stderr, "%s:%zu: INFO: processing object `"SV_Fmt"`\n", input_file_path, line_number, SV_Arg(name));
                one_object_encountered = true;
                one_object_line_number = line_number;
            } else if (sv_eq(kind, sv_from_cstr("s"))) {
                fprintf(stderr, "%s:%zu: WARNING: smooth groups are not supported right now. Ignoring them...\n", input_file_path, line_number);
            } else {
                fprintf(stderr, "%s:%zu: unknown kind of entry `"SV_Fmt"`\n", input_file_path, line_number, SV_Arg(kind));
                return_defer(1);
            }
        }
    }
    printf("Input:                  %s\n", input_file_path);
    printf("Output:                 %s\n", output_file_path);
    if (vertices.count > 0) 
        printf("Vertices:               %zu (coordinates x: %f..%f, y: %f..%f, z: %f..%f)\n", vertices.count, lx, hx, ly, hy, lz, hz);
    if (vnormals.count > 0) 
        printf("Normals:                %zu\n", vnormals.count);
    if (tcoords.count > 0) 
        printf("Texture Coordinates:    %zu (u: %f..%f, v: %f..%f)\n", tcoords.count, ltx, htx, lty, hty);
    if (faces.count > 0) 
        printf("Faces:                  %zu (vertices index: %d..%d)\n", faces.count, lf, hf);

    for (size_t i = 0; i < vertices.count; ++i) {
        vertices.items[i] = remap_object(vertices.items[i], scale, lx, hx, ly, hy, lz, hz);
    }

    FILE *out = fopen(output_file_path, "wb");
    if (out == NULL) {
        fprintf(stderr, "ERROR: Could not write file %s: %s\n", output_file_path, strerror(errno));
        return_defer(1);
    }
    generate_code(out, vertices, vnormals, tcoords, faces);

defer:
    return result;
}
