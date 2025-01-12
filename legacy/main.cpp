#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <iostream>
#include "types.h"

#define MAP_SIZE 8
static u8 MAPDATA[MAP_SIZE * MAP_SIZE] = {
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 3, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 2, 0, 4, 4, 0, 1,
    1, 0, 0, 0, 4, 0, 0, 1,
    1, 0, 3, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
};

const u32 HEIGHT = 216;
const u32 WIDTH = 384;

class Screen {
    public:
    SDL_Event event;
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    u32 pixels[WIDTH * HEIGHT];
    bool quit;
    v2 pos, dir, plane;

    Screen(const char* title) {
        window = SDL_CreateWindow(
                title,
                SDL_WINDOWPOS_CENTERED_DISPLAY(0),
                SDL_WINDOWPOS_CENTERED_DISPLAY(0),
                1280,
                720,
                SDL_WINDOW_ALLOW_HIGHDPI);
        ASSERT(window, "failed to create SDL window %s\n", SDL_GetError());

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
        ASSERT(renderer, "failed to create SDL renderer%s\n", SDL_GetError());

        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
        ASSERT(texture, "failed to create SDL texture%s\n", SDL_GetError());
        SDL_RenderSetScale(renderer, 1, 1);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 100);
        SDL_RenderClear(renderer);
        pos = (v2) { 2, 2 };
        dir = normalize(((v2) { -1.0f, 0.1f }));
        plane = (v2) { 0.0f, 0.66f };
        quit = false;
    }

    void verline(int x, int y0, int y1, u32 color) {
        for(int y = y0; y <= y1; y++) {
            pixels[(y * WIDTH) + x] = color;
        }
    }

    void render() {
        printf("POS: (%f, %f)\n", pos.x, pos.y);
        printf("DIR: (%f, %f)\n", dir.x, dir.y);
        for (int x = 0; x < WIDTH; x++) {
            const f32 xcam = (2 * (x / (f32) (WIDTH))) - 1;

            const v2 direction = {
                dir.x + plane.x * xcam,
                dir.y + plane.y * xcam
            };
            v2 position = pos;
            v2i iposition = { (int) pos.x, (int) pos.y };

            f32 distx = fabsf(direction.x) < 1e-20 ? 1e30 : fabsf(1.0f / direction.x);
            f32 disty = fabsf(direction.y) < 1e-20 ? 1e30 : fabsf(1.0f / direction.y);
            const v2 deltadist = { distx, disty };

            v2 sidedist = {
                deltadist.x * (direction.x < 0 ? (position.x - iposition.x) : (iposition.x + 1 - position.x)),
                deltadist.y * (direction.y < 0 ? (position.y - iposition.y) : (iposition.y + 1 - position.y)),
            };

            const v2i step = { (int) sign(direction.x), (int) sign(direction.y) };

            struct { int val, side; v2 pos; } hit = { 0, 0, { 0.0f, 0.0f } };

            while (!hit.val) {
                if (sidedist.x < sidedist.y) {
                    sidedist.x += deltadist.x;
                    iposition.x += step.x;
                    hit.side = 0;
                } else {
                    sidedist.y += deltadist.y;
                    iposition.y += step.y;
                    hit.side = 1;
                }
                ASSERT(
                        iposition.x >= 0 &&
                        iposition.x < MAP_SIZE &&
                        iposition.y >= 0 &&
                        iposition.y < MAP_SIZE,
                        "DDA Out of Bounds!");
                hit.val = MAPDATA[iposition.y * MAP_SIZE + iposition.x];

            }

            u32 color;
            switch (hit.val) {
                case 1: color = 0xff0000ff; break;
                case 2: color = 0xff00ff00; break;
                case 3: color = 0xffff0000; break;
                case 4: color = 0xffff00ff; break;
            }

            if(hit.side == 1) {
                const u32 br = ((color & 0xff00ff) * 0xc0) >> 8;
                const u32 g = ((color & 0x00ff00) * 0xc0) >> 8;

                color = 0xff000000 | (br & 0xff00ff) | (g & 0x00ff00);
            }

            hit.pos = (v2) { position.x + sidedist.x, position.y + sidedist.y };

            const f32 dperp = hit.side == 0 ? (sidedist.x - deltadist.x) : (sidedist.y - deltadist.y);

            const int h = (int) (HEIGHT / dperp);
            const int y0 = max((HEIGHT / 2) - (h / 2), 0);
            const int y1 = min((HEIGHT / 2) + (h / 2), HEIGHT - 1);

            verline(x, 0, y0, 0xff202020);
            verline(x, y0, y1, color);
            verline(x, y1, HEIGHT - 1, 0xff505050);
        }
    }
    void rotate(f32 rot) {
        const v2 d = dir, p = plane;
        dir.x = d.x * cos(rot) - d.y * sin(rot);
        dir.y = d.x * sin(rot) + d.y * cos(rot);
        plane.x = p.x * cos(rot) - p.y * sin(rot);
        plane.y = p.x * cos(rot) + p.y * cos(rot);

    }
    void loop() {
        while (!quit) {
            while(SDL_PollEvent(&event)) {
                switch (event.type) {
                    case SDL_QUIT:
                        quit = true;
                        break;
                }
            }
            const f32 rotspeed = 3.0f * 0.016f;
            const f32 movespeed = 3.0f * 0.016f;
            const u8 *keystate = SDL_GetKeyboardState(NULL);
            if(keystate[SDL_SCANCODE_LEFT]) {
                rotate(+rotspeed);
            }
            if(keystate[SDL_SCANCODE_RIGHT]) {
                rotate(-rotspeed);
            }
            if(keystate[SDL_SCANCODE_UP]) {
                pos.x += dir.x * movespeed;
                pos.y += dir.y * movespeed;
            }

            if(keystate[SDL_SCANCODE_DOWN]) {
                pos.x -= dir.x * movespeed;
                pos.y -= dir.y * movespeed;
            }

            memset(pixels, 0, sizeof(pixels));
            render();

            SDL_UpdateTexture(texture, NULL, pixels, WIDTH * 4);
            SDL_RenderCopyEx(renderer, texture, NULL, NULL, 0.0, NULL, SDL_FLIP_VERTICAL);
            SDL_RenderPresent(renderer);
        }
    }

    ~Screen() {
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
    }
};

int main() {
    ASSERT(!SDL_Init(SDL_INIT_VIDEO), "Error initializing SDL: %s\n", SDL_GetError());

    Screen window("engine");

    window.loop();


    return 0;
}
