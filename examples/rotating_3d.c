/*
    This example renders a rotating dot matrix with 3D blending.
    This idea is that you can take this code and compile it to different platforms with different rendering machanisms:
    native with SDL, WebAssembly with HTML5 canvas, etc.
*/
#define OLIVEC_IMPLEMENTATION
#include "olive.c"

float sqrtf(float x);
float atan2f(float y, float x);
float sinf(float x);
float cosf(float x);
#define PI 3.14159265359

#define WIDTH 800
#define HEIGHT 600
#define BACKGROUND_COLOR 0xFF181818
#define CIRCLE_COLOR 0xFF2020AA
#define PAGS 8
#define COLS 8
#define ROWS 8
#define GRID_INTERVAL 0.1
#define CIRCLE_RADIUS 5
#define Z_OFFSET 0.5

static uint32_t pixels[WIDTH*HEIGHT];

// TODO: Specify the origin, axis, and angle to achieve spatial rotation
static inline void rotate_point(float *x, float *y, float cx, float cy, float beta)
{
    float xt = *x - cx;
    float yt = *y - cy;

    *x = cosf(beta)*xt - sinf(beta)*yt + cx;
    *y = sinf(beta)*xt + cosf(beta)*yt + cy;
}

float theta = 0;

uint32_t *render(float dt)
{
    theta += dt*PI*0.25;

    Olivec_Canvas oc = olivec_canvas(pixels, WIDTH, HEIGHT);
    olivec_fill(oc, BACKGROUND_COLOR);

    for (int k = 0; k < PAGS; k++) {
        for (int i = 0; i < COLS; i++) {
            for (int j = 0; j < ROWS; j++) {
                float x = i*GRID_INTERVAL - (COLS-1)*GRID_INTERVAL/2;
                float y = j*GRID_INTERVAL - (ROWS-1)*GRID_INTERVAL/2;
                float z = k*GRID_INTERVAL + Z_OFFSET;

                float cx = 0;
                float cy = 0;
                float cz = Z_OFFSET + (PAGS - 1)*GRID_INTERVAL/2;
                
                rotate_point(&x, &y, cx, cy, theta);
                rotate_point(&y, &z, cy, cz, theta);
                rotate_point(&x, &z, cx, cz, theta);

                uint32_t r = i*255/COLS;
                uint32_t g = j*255/ROWS;
                uint32_t b = k*255/PAGS;
                uint32_t color = 0xFF000000 | (r<<(0*8)) | (g<<(1*8)) | (b<<(2*8));

                olivec_fill_circle(oc, x/z*HEIGHT/2 + WIDTH/2, (y/z + 1)*HEIGHT/2, CIRCLE_RADIUS, color);                
            }
        }
    }
    return pixels;
}

#ifdef SDL_PLATFORM
#include <stdio.h>
#include <SDL2/SDL.h>

#define return_defer(value) do { result = (value); goto defer; } while (0)

int main(void)
{
    int result = 0;

    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Texture *texture = NULL;

    {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) return_defer(1);

        window = SDL_CreateWindow("Olivec", 0, 0, WIDTH, HEIGHT, 0);
        if (window == NULL) return_defer(1);

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (renderer == NULL) return_defer(1);

        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
        if (texture == NULL) return_defer(1);

        Uint32 prev = SDL_GetTicks();
        for (;;) {
            // Compute Delta Time
            Uint32 curr = SDL_GetTicks();
            float dt = (curr - prev)/1000.f;
            prev = curr;

            // Flush the events
            SDL_Event event;
            while (SDL_PollEvent(&event)) if (event.type == SDL_QUIT) return_defer(0);

            // Render the texture
            SDL_Rect window_rect = {0, 0, WIDTH, HEIGHT};
            uint32_t *pixels_src = render(dt);
            void *pixels_dst;
            int pitch;
            if (SDL_LockTexture(texture, &window_rect, &pixels_dst, &pitch) < 0) return_defer(1);
            for (size_t y = 0; y < HEIGHT; ++y) {
                memcpy(pixels_dst + y*pitch, pixels_src + y*WIDTH, WIDTH*sizeof(uint32_t));
            }
            SDL_UnlockTexture(texture);

            // Display the texture
            if (SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0) < 0) return_defer(1);
            if (SDL_RenderClear(renderer) < 0) return_defer(1);
            if (SDL_RenderCopy(renderer, texture, &window_rect, &window_rect) < 0) return_defer(1);
            SDL_RenderPresent(renderer);
        }
    }

defer:
    switch (result) {
    case 0:
        printf("OK\n");
        break;
    default:
        fprintf(stderr, "SDL ERROR: %s\n", SDL_GetError());
    }
    if (texture) SDL_DestroyTexture(texture);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
    return result;
}
#endif // SDL_PLATFORM
