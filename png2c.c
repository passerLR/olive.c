#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define STB_IMAGE_IMPLEMENTATION
#include "./stb_image.h"

const char *shift(int *argc, char ***argv)
{
    assert(*argc > 0);
    const char *result = *argv[0];
    *argc -= 1;
    *argv += 1;
    return result;
}

int main(int argc, char *argv[])
{
    shift(&argc, &argv);        // skip program name

    if (argc <= 0) {
        fprintf(stderr, "Usage: png2c <filepath.png>\n");
        fprintf(stderr, "ERROR: expected file path\n");
        exit(1);
    }

    const char *filepath = shift(&argc, &argv);

    int x, y, n;
    uint32_t *data = (uint32_t *)stbi_load(filepath, &x, &y, &n, 4);

    if (data == NULL) {
        fprintf(stderr, "Could not load file `%s`\n", filepath);
        exit(1);
    }

    const char *outputpath = shift(&argc, &argv);
    FILE *f = fopen(outputpath, "w");
    // TODO: inclusion guards and the array name are not customizable
    fprintf(f, "#ifndef PNG_H_\n");
    fprintf(f, "#define PNG_H_\n");
    // fprintf(f, "#include <stdint.h>\n");
    fprintf(f, "size_t png_width = %d;\n", x);
    fprintf(f, "size_t png_height = %d;\n", y);
    fprintf(f, "uint32_t png[] = {");
    for (size_t i = 0; i < (size_t)(x * y); ++i) {
        fprintf(f, "0x%x, ", data[i]);
    }
    fprintf(f, "};\n");
    fprintf(f, "#endif // PNG_H_\n");
    fclose(f);

    return 0;
}