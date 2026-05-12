#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#define return_defer(value) do { result = (value); goto defer; } while (0)

#define UNUSED(x) (void)(x)

#define UNIMPLEMENTED(message) \
    do { \
        fprintf(stderr, "%s:%d: UNIMPLEMENTED: %s\n", __FILE__, __LINE__, message); \
        exit(1); \
    } while (0)

#define UNREACHABLE(message) \
    do { \
        fprintf(stderr, "%s:%d: UNREACHABLE: %s\n", __FILE__, __LINE__, message); \
        exit(1); \
    } while (0)


#define ARENA_IMPLEMENTATION
#include "arena.h"

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

#define STBI_MALLOC context_alloc
#define STBI_FREE UNUSED
#define STBI_REALLOC_SIZED context_realloc
#define STB_IMAGE_IMPLEMENTATION
#include "./stb_image.h"
#define STBIW_MALLOC STBI_MALLOC
#define STBIW_FREE STBI_FREE
#define STBIW_REALLOC_SIZED STBI_REALLOC_SIZED
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "./stb_image_write.h"

#define OLIVEC_IMPLEMENTATION
#include "olive.c"

#define BACKGROUND_COLOR 0xFF202020
#define FOREGROUND_COLOR 0xFF0000FF
#define RED_COLOR   0xFF2020AA
#define GREEN_COLOR 0xFF20AA20
#define BLUE_COLOR  0xFFAA2020
#define ERROR_COLOR 0xFFFF00FF

#define TEST_DIR_PATH "./test"

bool record_test_case(Olivec_Canvas oc, const char *file_path)
{
    if (!stbi_write_png(file_path, oc.width, oc.height, 4, oc.pixels, sizeof(uint32_t)*oc.stride)) {
        fprintf(stderr, "ERROR: could not write file %s: %s\n", file_path, strerror(errno));
        return(false);
    }
    printf("Generated %s\n", file_path);
    return(true);
}
typedef enum {
    REPLAY_PASSED,
    REPLAY_FAILED,
    REPLAY_ERRORED,
} Replay_Result;

Replay_Result replay_test_case(const char *program_path, Olivec_Canvas act_oc, const char *expected_file_path, const char *actual_file_path, const char *diff_file_path)
{
    int expected_width, expected_height;
    uint32_t *expected_pixels = (uint32_t*) stbi_load(expected_file_path, &expected_width, &expected_height, NULL, 4);
    if (expected_pixels == NULL) {
        fprintf(stderr, "%s: ERROR: could not read the file: %s\n", expected_file_path, strerror(errno));
        if (errno == ENOENT) {
            fprintf(stderr, "%s: HINT: Consider running `$ %s record` to create it\n", expected_file_path, program_path);
        }
        return(REPLAY_ERRORED);
    }

    Olivec_Canvas ext_oc = olivec_canvas(expected_pixels, expected_width, expected_height, expected_width);

     if (ext_oc.width != act_oc.width || ext_oc.height != act_oc.height) {
        fprintf(stderr, "%s: TEST FAILURE: unexpected image size. Expected %dx%d, but got %zux%zu\n", expected_file_path, expected_width, expected_height, act_oc.width, act_oc.height);
        return(REPLAY_FAILED);
    }

    uint32_t *diff_pixels = context_alloc(sizeof(uint32_t)*act_oc.width*act_oc.height);
    Olivec_Canvas diff_canvas = olivec_canvas(diff_pixels, act_oc.width, act_oc.height, act_oc.width);

    bool failed = false;
    for (size_t y = 0; y < act_oc.height; ++y) {
        for (size_t x = 0; x < act_oc.width; ++x) {
            uint32_t expected_pixel = OLIVEC_PIXEL(ext_oc, x, y);
            uint32_t actual_pixel = OLIVEC_PIXEL(act_oc, x, y);
            if (expected_pixel != actual_pixel) {
                OLIVEC_PIXEL(diff_canvas, x, y) = ERROR_COLOR;
                failed = true;
            } else {
                OLIVEC_PIXEL(diff_canvas, x, y) = expected_pixel;
            }
        }
    }

    if (failed) {
        fprintf(stderr, "%s: TEST FAILURE: unexpected pixels in generated image\n", expected_file_path);
        if (!stbi_write_png(actual_file_path, act_oc.width, act_oc.height, 4, act_oc.pixels, sizeof(uint32_t)*act_oc.stride)) {
            fprintf(stderr, "ERROR: could not write actual image file %s: %s\n", actual_file_path, strerror(errno));
            return(REPLAY_ERRORED);
        }
        if (!stbi_write_png(diff_file_path, diff_canvas.width, diff_canvas.height, 4, diff_canvas.pixels, sizeof(uint32_t)*diff_canvas.stride)) {
            fprintf(stderr, "ERROR: could not write diff image file %s: %s\n", diff_file_path, strerror(errno));
            return(REPLAY_ERRORED);
        }
        fprintf(stderr, "%s: HINT: See actual image %s\n", expected_file_path, actual_file_path);
        fprintf(stderr, "%s: HINT: See diff image %s\n", expected_file_path, diff_file_path);
        fprintf(stderr, "%s: HINT: If this behaviour is intentional confirm that by updating the image with `$ %s record`\n", expected_file_path, program_path);
        return(REPLAY_FAILED);
    }
    printf("%s OK\n", expected_file_path);

    return(REPLAY_PASSED);

}


typedef struct {
    Olivec_Canvas (*generate_actual_canvas)(void);
    const char *expected_file_path;
    const char *actual_file_path;
    const char *diff_file_path;
} Test_Case;

#define DEFINE_TEST_CASE(name) \
    { \
        .generate_actual_canvas = name, \
        .expected_file_path = TEST_DIR_PATH "/" #name "_expected.png", \
        .actual_file_path = TEST_DIR_PATH "/" #name "_actual.png", \
        .diff_file_path = TEST_DIR_PATH "/" #name "_diff.png", \
    }


#define WIDTH 128
#define HEIGHT 128

Olivec_Canvas test_fill_rect(void)
{
    uint32_t *pixels = context_alloc(WIDTH*HEIGHT*sizeof(uint32_t));
    Olivec_Canvas oc = olivec_canvas(pixels, WIDTH, HEIGHT, WIDTH);
    olivec_fill(oc, BACKGROUND_COLOR);
    olivec_fill_rect(oc, WIDTH/2 - WIDTH/8, HEIGHT/2 - HEIGHT/8, WIDTH/4, HEIGHT/4, RED_COLOR);
    olivec_fill_rect(oc, WIDTH - 1, HEIGHT - 1, -WIDTH/2, -HEIGHT/2, GREEN_COLOR);
    olivec_fill_rect(oc, -WIDTH/4, -HEIGHT/4, WIDTH/2, HEIGHT/2, BLUE_COLOR);

    return oc;
}

Olivec_Canvas test_fill_circle(void)
{
    uint32_t *pixels = context_alloc(WIDTH*HEIGHT*sizeof(uint32_t));
    Olivec_Canvas oc = olivec_canvas(pixels, WIDTH, HEIGHT, WIDTH);
    olivec_fill(oc, BACKGROUND_COLOR);
    olivec_fill_circle(oc, 0, 0, WIDTH/2, RED_COLOR);
    olivec_fill_circle(oc, WIDTH/2, HEIGHT/2, WIDTH/4, BLUE_COLOR);
    olivec_fill_circle(oc, WIDTH*3/4, HEIGHT*3/4, -WIDTH/4, GREEN_COLOR);

    return oc;
}

Olivec_Canvas test_draw_line(void)
{
    uint32_t *pixels = context_alloc(WIDTH*HEIGHT*sizeof(uint32_t));
    Olivec_Canvas oc = olivec_canvas(pixels, WIDTH, HEIGHT, WIDTH);
    olivec_fill(oc, BACKGROUND_COLOR);
    olivec_draw_line(oc, 0, 0, WIDTH, HEIGHT, RED_COLOR);
    olivec_draw_line(oc, WIDTH, 0, 0, HEIGHT, GREEN_COLOR);

    return oc;
}

Olivec_Canvas test_fill_triangle(void)
{
    uint32_t *pixels = context_alloc(WIDTH*HEIGHT*sizeof(uint32_t));
    Olivec_Canvas oc = olivec_canvas(pixels, WIDTH, HEIGHT, WIDTH);
    olivec_fill(oc, BACKGROUND_COLOR);
    {
        int x1 = WIDTH/2, y1 = HEIGHT/8;
        int x2 = WIDTH/8, y2 = HEIGHT/2;
        int x3 = WIDTH*7/8, y3 = HEIGHT*7/8;
        olivec_fill_triangle(oc, x1, y1, x2, y2, x3, y3, RED_COLOR);
    }
    {
        int x1 = WIDTH/2, y1 = HEIGHT*2/8;
        int x2 = WIDTH*2/8, y2 = HEIGHT/2;
        int x3 = WIDTH*6/8, y3 = HEIGHT/2;
        olivec_fill_triangle(oc, x1, y1, x2, y2, x3, y3, GREEN_COLOR);
    }
    {
        int x1 = WIDTH*4/8, y1 = HEIGHT*3/8;
        int x2 = WIDTH*4/8, y2 = HEIGHT*5/8;
        int x3 = WIDTH*6/8, y3 = HEIGHT*5/8;
        olivec_fill_triangle(oc, x1, y1, x2, y2, x3, y3, BLUE_COLOR);
    }
    {
        int x1 = WIDTH*2/8, y1 = HEIGHT/8;
        int x2 = WIDTH*2/8, y2 = HEIGHT*3/8;
        int x3 = WIDTH*1/8, y3 = HEIGHT*3/8;
        uint32_t color = 0xFFFFFFFF;
        olivec_fill_triangle(oc, x1, y1, x2, y2, x3, y3, color);
    }

    return oc;
}

Olivec_Canvas test_alpha_blending(void)
{
    uint32_t *pixels = context_alloc(WIDTH*HEIGHT*sizeof(uint32_t));
    Olivec_Canvas oc = olivec_canvas(pixels, WIDTH, HEIGHT, WIDTH);
    olivec_fill(oc, BACKGROUND_COLOR);
    olivec_fill_rect(oc, 0, 0, WIDTH*3/4, HEIGHT*3/4, RED_COLOR);
    olivec_fill_rect(oc, WIDTH - 1, HEIGHT - 1, -WIDTH*3/4, -HEIGHT*3/4, 0x7720AA20);
    olivec_fill_circle(oc, WIDTH/2, HEIGHT/2, WIDTH*3/8, 0xBBAA2020);
    {
        int x1 = WIDTH/2, y1 = HEIGHT/8;
        int x2 = WIDTH/8, y2 = HEIGHT/2;
        int x3 = WIDTH*7/8, y3 = HEIGHT*7/8;
        olivec_fill_triangle(oc, x1, y1, x2, y2, x3, y3, 0x7720AAAA);
    }

    return oc;
}

Olivec_Canvas checker_example(void)
{
    int width  = 800;
    int height = 600;
    int cols = (8*2);
    int rows = (6*2);
    int cell_width = (width/cols);
    int cell_height = (height/rows);
    uint32_t *pixels = context_alloc(width*height*sizeof(uint32_t));
    Olivec_Canvas oc = olivec_canvas(pixels, width, height, width);
    olivec_fill(oc, BACKGROUND_COLOR);

    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            uint32_t color;
            if ((x + y)%2 == 0) {
                color = BACKGROUND_COLOR;
            } else {
                color = FOREGROUND_COLOR;
            }
            olivec_fill_rect(oc, x*cell_width, y*cell_height, cell_width, cell_height, color);
        }
    }
    return oc;
}

Olivec_Canvas circle_example(void)
{
    int width  = 800;
    int height = 600;
    int cols = (8*2);
    int rows = (6*2);
    int cell_width = (width/cols);
    int cell_height = (height/rows);
    uint32_t *pixels = context_alloc(width*height*sizeof(uint32_t));
    Olivec_Canvas oc = olivec_canvas(pixels, width, height, width);
    olivec_fill(oc, BACKGROUND_COLOR);

    int radius = cell_width < cell_height ? cell_width : cell_height;
    radius >>= 1;
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            olivec_fill_circle(oc, x*cell_width + cell_width/2, y*cell_height + cell_height/2, -radius/8*(1 + 7*(x/(cols*2.0) + y/(rows*2.0))), FOREGROUND_COLOR);
        }
    }
    return oc;
}

Olivec_Canvas line_example(void)
{
    int width  = 800;
    int height = 600;
    uint32_t *pixels = context_alloc(width*height*sizeof(uint32_t));
    Olivec_Canvas oc = olivec_canvas(pixels, width, height, width);
    olivec_fill(oc, BACKGROUND_COLOR);
    olivec_draw_line(oc, 0, 0, width, height, FOREGROUND_COLOR);
    olivec_draw_line(oc, width, 0, 0, height, FOREGROUND_COLOR);
    olivec_draw_line(oc, 0, 0, width/4, height, 0xFF00FF00);
    olivec_draw_line(oc, width/4, 0, 0, height, 0xFF00FF00);
    olivec_draw_line(oc, 0, height/2, width, height/2, 0xFFFF0000);
    olivec_draw_line(oc, width/2, 0, width/2, height,  0xFFFF0000);
    return oc;
}

Olivec_Canvas text_example(void)
{
    int width  = 800;
    int height = 600;
    uint32_t *pixels = context_alloc(width*height*sizeof(uint32_t));
    Olivec_Canvas oc = olivec_canvas(pixels, width, height, width);
    olivec_fill(oc, BACKGROUND_COLOR);
    const char text[] = "the quick brown fox jumps over the lazy dog.";
    const char lowercase[] = "abcdefghijklmnopqrstuvwxyz";
    const char uppercase[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const char number[]    = "01234567890123456789";
    const char symbols1[]  = ". , : ; \" \' ! ? + - * / % = ~ |";
    const char symbols2[]  = "@ # $ _ ^ \\ & < > ( ) [ ] { }";
    olivec_text(oc, text, default_font, 0, 0, 2, 0xFFFFFFFF);
    olivec_text(oc, lowercase, default_font, 0, height*1/8, 3, 0xFFFFFFFF);
    olivec_text(oc, uppercase, default_font, 0, height*2/8, 3, 0xFFFFFFFF);
    olivec_text(oc, number,    default_font, 0, height*3/8, 3, 0xFFFFFFFF);
    olivec_text(oc, symbols1,  default_font, 0, height*4/8, 3, 0xFFFFFFFF);
    olivec_text(oc, symbols2,  default_font, 0, height*5/8, 3, 0xFFFFFFFF);
    return oc;
}

Olivec_Canvas kun_example(void)
{
    const char *filepath = "./assets/pig.png";
    int x, y, n;
    uint32_t *data = (uint32_t *)stbi_load(filepath, &x, &y, &n, 4);
    if (data == NULL) {
        fprintf(stderr, "Could not load file `%s`: %s\n", filepath, strerror(errno));
        return olivec_canvas(NULL, 0, 0, 0);
    }
    printf("%d, %d, %d\n", x, y, n);

    int width  = 800;
    int height = 600;
    uint32_t *pixels = context_alloc(width*height*sizeof(uint32_t));
    olivec_fill(olivec_canvas(pixels, width, height, width), 0xFF0000FF);
    assert(x < width);
    assert(y < height);
    Olivec_Canvas oc = olivec_canvas(pixels, width, height, width);
    olivec_copy(oc, olivec_canvas(data, x, y, x));
    return oc;
}


Test_Case test_cases[] = {
    DEFINE_TEST_CASE(test_fill_rect),
    DEFINE_TEST_CASE(test_fill_circle),
    DEFINE_TEST_CASE(test_draw_line),
    DEFINE_TEST_CASE(test_fill_triangle),
    DEFINE_TEST_CASE(test_alpha_blending),
    DEFINE_TEST_CASE(checker_example),
    DEFINE_TEST_CASE(circle_example),
    DEFINE_TEST_CASE(line_example),
    DEFINE_TEST_CASE(text_example),
    DEFINE_TEST_CASE(kun_example),
};

#define TEST_CASES_COUNT (sizeof(test_cases)/sizeof(test_cases[0]))

int main(int argc, char **argv)
{
    int result = 0;
    assert(argc >= 1);
    const char *program_path = argv[0];
    bool record = argc >= 2 && strcmp(argv[1], "record") == 0;

    for (size_t i = 0; i < TEST_CASES_COUNT; ++i) {
        Olivec_Canvas actual_canvas = test_cases[i].generate_actual_canvas();
        if (record) {
            if (!record_test_case(actual_canvas, test_cases[i].expected_file_path)) return_defer(1);
        } else {
            if (replay_test_case(program_path, actual_canvas, test_cases[i].expected_file_path, test_cases[i].actual_file_path, test_cases[i].diff_file_path) == REPLAY_ERRORED) return_defer(1);
        }
        arena_reset(&default_arena);
    }

defer:
    arena_free(&default_arena);
    return result;
}

