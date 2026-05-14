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
#define WHITE_COLOR 0xFFAAAAAA
#define RED_COLOR   0xFF2020AA
#define GREEN_COLOR 0xFF20AA20
#define BLUE_COLOR  0xFFAA2020
#define ERROR_COLOR 0xFFFF00FF

#define TEST_DIR_PATH "./test"

bool canvas_load(const char *file_path, Olivec_Canvas *oc)
{
    int width, height;
    uint32_t *pixels = (uint32_t*) stbi_load(file_path, &width, &height, NULL, 4);
    if (pixels == NULL) return false;
    *oc = olivec_canvas(pixels, width, height, width);
    return true;
}

bool canvas_save(Olivec_Canvas oc, const char *file_path)
{
    return stbi_write_png(file_path, oc.width, oc.height, 4, oc.pixels, sizeof(uint32_t)*oc.stride);
}

typedef struct {
    Olivec_Canvas (*generate_actual_canvas)(void);
    const char *id;
    const char *expected_file_path;
    const char *actual_file_path;
    const char *diff_file_path;
} Test_Case;

#define DEFINE_TEST_CASE(name) \
    { \
        .generate_actual_canvas = name, \
        .id = #name, \
        .expected_file_path = TEST_DIR_PATH "/" #name "_expected.png", \
        .actual_file_path = TEST_DIR_PATH "/" #name "_actual.png", \
        .diff_file_path = TEST_DIR_PATH "/" #name "_diff.png", \
    }

bool update_test_case(const Test_Case *tc)
{
    Olivec_Canvas oc = tc->generate_actual_canvas();
    const char *file_path = tc->expected_file_path;

    if (!canvas_save(oc, file_path)) {
        fprintf(stderr, "ERROR: could not write file %s: %s\n", file_path, strerror(errno));
        return(false);
    }
    printf("%s: Generated %s\n", tc->id, file_path);
    return(true);
}

Olivec_Canvas canvas_alloc(size_t width, size_t height)
{
    uint32_t *pixels = context_alloc(sizeof(uint32_t)*width*height);
    return olivec_canvas(pixels, width, height, width);
}

typedef enum {
    REPLAY_PASSED,
    REPLAY_FAILED,
    REPLAY_ERRORED,
} Replay_Result;

static inline size_t min_size(size_t a, size_t b)
{
    if (a < b) return a;
    return b;
}

static inline size_t max_size(size_t a, size_t b)
{
    if (a > b) return a;
    return b;
}

Replay_Result run_test_case(const char *program_path, const Test_Case *tc)
{
    printf("%s:", tc->id);
    fflush(stdout);
    
    const char *expected_file_path = tc->expected_file_path;
    const char *actual_file_path = tc->actual_file_path;
    const char *diff_file_path = tc->diff_file_path;
    Olivec_Canvas act_oc = tc->generate_actual_canvas();
    Olivec_Canvas ext_oc;
    if (!canvas_load(expected_file_path, &ext_oc)) {
        fprintf(stderr, "\n");
        fprintf(stderr, "  ERROR: could not read %s: %s\n", expected_file_path, stbi_failure_reason());
        if (errno == ENOENT) {
            fprintf(stderr, "  HINT: Consider running `$ %s update %s` to create it\n", program_path, tc->id);
        }
        return(REPLAY_ERRORED);
    }

    bool failed = false;
    if (ext_oc.width != act_oc.width || ext_oc.height != act_oc.height) {
        failed = true;
    }

    Olivec_Canvas diff_canvas = canvas_alloc(max_size(ext_oc.width, act_oc.width), max_size(ext_oc.height, act_oc.height));
    olivec_fill(diff_canvas, ERROR_COLOR);

    for (size_t y = 0; y < min_size(ext_oc.height, act_oc.height); ++y) {
        for (size_t x = 0; x < min_size(ext_oc.width, act_oc.width); ++x) {
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
        fprintf(stderr, "\n");
        if (!canvas_save(act_oc, actual_file_path)) {
            fprintf(stderr, "  ERROR: could not write image file with actual pixels %s: %s\n", actual_file_path, strerror(errno));
            return(REPLAY_ERRORED);
        }
        if (!canvas_save(diff_canvas, diff_file_path)) {
            fprintf(stderr, "  ERROR: could not wrilte diff image file %s: %s\n", diff_file_path, strerror(errno));
            return(REPLAY_ERRORED);
        }
        fprintf(stderr, "  TEST FAILURE: unexpected pixels in generated image\n");
        fprintf(stderr, "  Expected:     %s\n", expected_file_path);
        fprintf(stderr, "  Actual:       %s\n", actual_file_path);
        fprintf(stderr, "  Diff:         %s\n", diff_file_path);
        fprintf(stderr, "  HINT: If this behaviour is intentional confirm that by updating the image with `$ %s update`\n", program_path);
        return(REPLAY_FAILED);
    }
    printf(" OK\n");

    return(REPLAY_PASSED);

}


#define WIDTH 128
#define HEIGHT 128

Olivec_Canvas test_fill_rect(void)
{
    Olivec_Canvas oc = canvas_alloc(WIDTH, HEIGHT);
    olivec_fill(oc, BACKGROUND_COLOR);
    olivec_fill_rect(oc, WIDTH/2 - WIDTH/8, HEIGHT/2 - HEIGHT/8, WIDTH/4, HEIGHT/4, RED_COLOR);
    olivec_fill_rect(oc, WIDTH - 1, HEIGHT - 1, -WIDTH/2, -HEIGHT/2, GREEN_COLOR);
    olivec_fill_rect(oc, -WIDTH/4, -HEIGHT/4, WIDTH/2, HEIGHT/2, BLUE_COLOR);

    return oc;
}

Olivec_Canvas test_fill_circle(void)
{
    Olivec_Canvas oc = canvas_alloc(WIDTH, HEIGHT);
    olivec_fill(oc, BACKGROUND_COLOR);
    olivec_fill_circle(oc, 0, 0, WIDTH/2, RED_COLOR);
    olivec_fill_circle(oc, WIDTH/2, HEIGHT/2, WIDTH/4, BLUE_COLOR);
    olivec_fill_circle(oc, WIDTH*3/4, HEIGHT*3/4, -WIDTH/4, GREEN_COLOR);

    return oc;
}

Olivec_Canvas test_draw_line(void)
{
    Olivec_Canvas oc = canvas_alloc(WIDTH, HEIGHT);
    olivec_fill(oc, BACKGROUND_COLOR);
    olivec_draw_line(oc, 0, 0, WIDTH, HEIGHT, RED_COLOR);
    olivec_draw_line(oc, WIDTH, 0, 0, HEIGHT, GREEN_COLOR);

    return oc;
}

Olivec_Canvas test_fill_triangle(void)
{
    Olivec_Canvas oc = canvas_alloc(WIDTH, HEIGHT);
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
    Olivec_Canvas oc = canvas_alloc(WIDTH, HEIGHT);
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
    int width  = 400;
    int height = 300;
    int cols = (8);
    int rows = (6);
    int cell_width = (width/cols);
    int cell_height = (height/rows);
    Olivec_Canvas oc = canvas_alloc(width, height);
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
    int width  = 400;
    int height = 300;
    int cols = (8);
    int rows = (6);
    int cell_width = (width/cols);
    int cell_height = (height/rows);
    Olivec_Canvas oc = canvas_alloc(width, height);
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
    int width  = 400;
    int height = 300;
    Olivec_Canvas oc = canvas_alloc(width, height);
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
    Olivec_Canvas oc = canvas_alloc(width, height);
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

Olivec_Canvas test_hello_world(void)
{
    int width  = 400;
    int height = 160;
    Olivec_Canvas oc = canvas_alloc(width, height);
    olivec_fill(oc, BACKGROUND_COLOR);
    const char text[] = "Hello World!";
    size_t glyph_size = 4;
    olivec_text(oc, text, default_font, width/2 - 6*(default_font.width + default_font.interval)*glyph_size, height/2 - default_font.height*glyph_size/2, glyph_size, 0xFFFFFFFF);
    return oc;
}

Olivec_Canvas kun_example(void)
{
    const char *filepath = "./assets/kun.png";
    int x, y, n;
    uint32_t *data = (uint32_t *)stbi_load(filepath, &x, &y, &n, 4);
    if (data == NULL) {
        fprintf(stderr, "Could not load file `%s`: %s\n", filepath, strerror(errno));
        return olivec_canvas(NULL, 0, 0, 0);
    }
    // printf("%d, %d, %d\n", x, y, n);

    size_t width  = 128;
    size_t height = 128;
    Olivec_Canvas oc = canvas_alloc(width, height);
    olivec_fill(oc, BACKGROUND_COLOR);
    olivec_sprite_copy(
        oc,
        olivec_canvas(data, x, y, x),
        0, 0, width, height);
        //width/2, height/2, width, height);

    return oc;
}

Olivec_Canvas test_line_edge_cases(void)
{
    size_t width = 10;
    size_t height = 10;
    Olivec_Canvas oc = canvas_alloc(width, height);
    olivec_fill(oc, BACKGROUND_COLOR);
    // One pixel line
    olivec_draw_line(oc, width/2, height/2, width/2, height/2, FOREGROUND_COLOR);
    // Out-of-bounds horizontally
    olivec_draw_line(oc, width + 10, height/2, width + 20, height/2, FOREGROUND_COLOR);
    // Out-of-bounds vertically
    olivec_draw_line(oc, width/2, height + 10, width/2, height + 20, FOREGROUND_COLOR);
    return oc;
}

Olivec_Canvas test_frame(void)
{
    size_t width = 128;
    size_t height = 128;
    Olivec_Canvas oc = canvas_alloc(width, height);
    olivec_fill(oc, BACKGROUND_COLOR);

    {
        size_t w = width/2;
        size_t h = width/2;
        olivec_frame(oc, 0, 0, w, h, 1, RED_COLOR);
        olivec_frame(oc, width/2, height/2, w, h, 1, GREEN_COLOR);
    }

    // Odd thiccness
    {
        size_t w = width/2;
        size_t h = width/2;
        size_t t = 5;
        olivec_frame(oc, width/2 - w/2, height/2 - h/2, w, h, t, WHITE_COLOR);
        olivec_frame(oc, width/2 - w/2, height/2 - h/2, w, h, 1, RED_COLOR);
    }

    // Even thiccness
    {
        size_t w = width/4;
        size_t h = width/4;
        size_t t = 6;
        olivec_frame(oc, width/2 - w/2, height/2 - h/2, w, h, t, WHITE_COLOR);
        olivec_frame(oc, width/2 - w/2, height/2 - h/2, w, h, 1, RED_COLOR);
    }

    // Zero thiccness
    {
        size_t w = width/8;
        size_t h = width/8;
        size_t t = 0;
        olivec_frame(oc, width/2 - w/2, height/2 - h/2, w, h, t, WHITE_COLOR);
    }

    return oc;
}

#include "./assets/ppng.c"
Olivec_Canvas test_copy_flip(void)
{
    size_t width = 128;
    size_t height = 128;
    Olivec_Canvas dst = canvas_alloc(width, height);
    Olivec_Canvas src = olivec_canvas(png_pixels, png_width, png_height, png_width);
    olivec_fill(dst, RED_COLOR);
    olivec_sprite_copy(dst, src, 0, 0, width/2, height/2);
    olivec_sprite_copy(dst, src, width - 1, 0, -width/2, height/2);
    olivec_sprite_copy(dst, src, 0, height - 1, width/2, -height/2);
    olivec_sprite_copy(dst, src, width - 1, height - 1, -width/2, -height/2);
    return dst;
}
Olivec_Canvas test_copy_flip_cut(void)
{
    size_t width = 128;
    size_t height = 128;
    Olivec_Canvas dst = canvas_alloc(width, height);
    Olivec_Canvas src = olivec_canvas(png_pixels, png_width, png_height, png_width);
    olivec_fill(dst, RED_COLOR);
    olivec_sprite_copy(dst, src, -width/2, -height/2, width, height);
    olivec_sprite_copy(dst, src, width - 1 + width/2, -height/2, -width, height);
    olivec_sprite_copy(dst, src, -width/2, height - 1 + height/2, width, -height);
    olivec_sprite_copy(dst, src, width - 1 + width/2, height - 1 + height/2, -width, -height);
    return dst;
}

#include "./assets/kun.c"
Olivec_Canvas test_sprite_blend_vs_copy(void)
{
    Olivec_Canvas sadge = olivec_canvas(kun_pixels, kun_width, kun_height, kun_width);
    size_t w = sadge.width;
    size_t h = sadge.height*2;
    Olivec_Canvas dst = canvas_alloc(w, h);
    olivec_fill(dst, RED_COLOR);
    olivec_sprite_blend(dst, sadge, 0, 0, sadge.width, sadge.height);
    olivec_sprite_copy(dst, sadge, 0, sadge.height, sadge.width, sadge.height);
    return dst;
}
Olivec_Canvas test_weird_triangle_bug(void)
{
    size_t w = 256;
    size_t h = 256;
    Olivec_Canvas dst = canvas_alloc(w, h);
    olivec_fill(dst, 0xFF181818);
    olivec_fill_triangle3c(dst, w/4, h/4, w/2, 0, 0, h, 0xFF0000FF, 0xFF00FF00, 0xFFFF0000);
    //olivec_fill_triangle3c(dst, w/4, h/4, 0, h-1, w-1, 0, 0xFF00FF00, 0xFFFF0000, 0xFF0000FF);
    return dst;
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
    DEFINE_TEST_CASE(test_hello_world),
    DEFINE_TEST_CASE(kun_example),
    DEFINE_TEST_CASE(test_line_edge_cases),
    DEFINE_TEST_CASE(test_frame),
    DEFINE_TEST_CASE(test_copy_flip),
    DEFINE_TEST_CASE(test_copy_flip_cut),
    DEFINE_TEST_CASE(test_sprite_blend_vs_copy),
    DEFINE_TEST_CASE(test_weird_triangle_bug),
};

#define TEST_CASES_COUNT (sizeof(test_cases)/sizeof(test_cases[0]))

const char *shift(int *argc, char ***argv)
{
    assert(*argc > 0);
    const char *result = *argv[0];
    *argc -= 1;
    *argv += 1;
    return result;
}

void list_available_tests(void)
{
    fprintf(stderr, "Available tests:\n");
    for (size_t i = 0; i < TEST_CASES_COUNT; ++i) {
        fprintf(stderr, "    %s\n", test_cases[i].id);
    }
}

Test_Case *find_test_case_by_id(const char *id)
{
    for (size_t i = 0; i < TEST_CASES_COUNT; ++i) {
        if (strcmp(test_cases[i].id, id) == 0) {
            return &test_cases[i];
        }
    }
    return NULL;
}

typedef struct {
    int (*run)(const char *program_path, int argc, char **argv);
    const char *id;
    const char *description;
} Subcmd;

void usage(const char *program_path);

int subcmd_run(const char *program_path, int argc, char **argv)
{
    if (argc <= 0) {
        for (size_t i = 0; i < TEST_CASES_COUNT; ++i) {
            if (run_test_case(program_path, &test_cases[i]) == REPLAY_ERRORED) return(1);
            arena_reset(&default_arena);
        }
    } else {
        const char *test_case_id = shift(&argc, &argv);
        Test_Case *tc = find_test_case_by_id(test_case_id);
        if (tc == NULL) {
            list_available_tests();
            fprintf(stderr, "ERROR: could not find test case `%s`\n", test_case_id);
            return(1);
        }

        if (run_test_case(program_path, tc) == REPLAY_ERRORED) return(1);
    }

    return 0;
}

int subcmd_update(const char *program_path, int argc, char **argv)
{
    UNUSED(program_path);

    if (argc <= 0) {
        for (size_t i = 0; i < TEST_CASES_COUNT; ++i) {
            if (!update_test_case(&test_cases[i])) return(1);
            arena_reset(&default_arena);
        }
    } else {
        const char *test_case_id = shift(&argc, &argv);
        Test_Case *tc = find_test_case_by_id(test_case_id);
        if (tc == NULL) {
            list_available_tests();
            fprintf(stderr, "ERROR: could not find test case `%s`\n", test_case_id);
            return(1);
        }

        if (!update_test_case(tc)) return(1);
    }

    return 0;
}

int subcmd_list(const char *program_path, int argc, char **argv)
{
    UNUSED(program_path);
    UNUSED(argc);
    UNUSED(argv);
    list_available_tests();
    return 0;
}

int subcmd_help(const char *program_path, int argc, char **argv)
{
    UNUSED(argc);
    UNUSED(argv);
    usage(program_path);
    return 0;
}

#define DEFINE_SUBCMD(name, desc) \
    { \
        .run = subcmd_##name, \
        .id = #name, \
        .description = desc, \
    }

Subcmd subcmds[] = {
    DEFINE_SUBCMD(run, "Run the tests"),
    DEFINE_SUBCMD(update, "Update the tests"),
    DEFINE_SUBCMD(list, "List all available tests"),
    DEFINE_SUBCMD(help, "Print this help message"),
};

#define SUBCMDS_COUNT (sizeof(subcmds)/sizeof(subcmds[0]))

Subcmd *find_subcmd_by_id(const char *id)
{
    for (size_t i = 0; i < SUBCMDS_COUNT; ++i) {
        if (strcmp(subcmds[i].id, id) == 0) {
            return &subcmds[i];
        }
    }
    return NULL;
}

void usage(const char *program_path)
{
    fprintf(stderr, "Usage: %s [Subcommand]\n", program_path);
    fprintf(stderr, "Subcommands:\n");

    int width = 0;
    for (size_t i = 0; i < SUBCMDS_COUNT; ++i) {
        int len = strlen(subcmds[i].id);
        if (width < len) width = len;
    }

    for (size_t i = 0; i < SUBCMDS_COUNT; ++i) {
        fprintf(stderr, "    %-*s - %s\n", width, subcmds[i].id, subcmds[i].description);
    }
}


int main(int argc, char **argv)
{
    int result = 0;

    {
        const char *program_path = shift(&argc,  &argv);

        if (argc <= 0) {
            usage(program_path);
            fprintf(stderr, "ERROR: no subcommand is provided\n");
            return_defer(1);
        }

        const char *subcmd_id = shift(&argc, &argv);
        Subcmd *subcmd = find_subcmd_by_id(subcmd_id);
        if (subcmd != NULL) {
            return_defer(subcmd->run(program_path, argc, argv));
        } else {
            usage(program_path);
            fprintf(stderr, "ERROR: unknown subcommand `%s`\n", subcmd_id);
            return_defer(1);
        }
    }

defer:
    arena_free(&default_arena);
    return result;
}

