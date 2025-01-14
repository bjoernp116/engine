#include <SDL2/SDL_events.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_video.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "types.h"


State state;

static void verline(int x, int y0, int y1, u32 color) {
    for (int y = y0; y <= y1; y++) {
        if (y > SCREEN_HEIGHT || x > SCREEN_WIDTH || y < 0 || x < 0) { ASSERT(false, "OUT OF BOUNDS!\n"); exit(-1); }
        state.pixels[(y * SCREEN_WIDTH) + x] = color;
    }
}

static inline v2 intersect(v2 a0, v2 a1, v2 b0, v2 b1) {
    const f32 d = ((a0.x - a1.x) * (b0.y - b1.y)) - ((a0.y - a1.y) * (b0.x - b1.x));

    if (fabsf(d) < 0.000001f) { return (v2) { NAN, NAN }; }

    const f32 t = (((a0.x - b0.x) * (b0.y - b1.y)) - ((a0.y - b0.y) * (b0.x - b1.x))) / d;
    const f32 u = (((a0.x - b0.x) * (a0.y - a1.y)) - ((a0.y - b0.y) * (a0.x - a1.x))) / d;

    return (t >= 0 && t <= 1 && u >= 0 && u <= 1) ?
        ((v2) {
            a0.x + (t * (a1.x - a0.x)),
            a0.y + (t * (a1.y - a0.y))
        }) : ((v2) {NAN, NAN} );

}

static void render() {
    for (int x = 0; x < SCREEN_WIDTH; x++) {
        // x coordinate in space from [-1, 1]
        const f32 xcam = (2 * (x / (f32) (SCREEN_WIDTH))) - 1;

        // ray direction through this column
        const v2 dir = {
            state.dir.x + state.plane.x * xcam,
            state.dir.y + state.plane.y * xcam
        };
        //printf("DIR: (%.f, %.f)", state.dir.x, state.dir.y);

        v2 pos = state.pos;

        // DDA hit
        struct { int val, side; v2 pos; } hit = { 0, 0, { 0.0f, 0.0f } };
        //printf("RAY_LENGTH: %f\n", length(ipos));
        /*while (!hit.val) {
            if (length(ipos) > 100) { break; }
            if (sidedist.x < sidedist.y) {
                sidedist.x += deltadist.x;
                ipos.x += step.x;
                hit.side = 0;
            } else {
                sidedist.y += deltadist.y;
                ipos.y += step.y;
                hit.side = 1;
            }


            for (usize i = 0; i < state.walls.n; i++) {
                Wall wall = state.walls.arr[i];
                v2 intersection = intersect(state.pos, ipos, wall.a, wall.b);
                if (!isnan(intersection.x)) {
                    hit.val = 1;
                    hit.pos = intersection;
                    break;
                } else {
                    hit.val = 0;
                }

            }
            //hit.val = MAPDATA[ipos.y * MAP_SIZE + ipos.x];
        }*/

        f32 min_distance = INFINITY;
        Wall* min_wall = NULL;
        v2* min_intersection = NULL;
        for (usize i = 0; i < state.walls.n; i++) {
            Wall wall = state.walls.arr[i];

            v2 p = { wall.b.x - wall.a.x, wall.b.y - wall.a.y };
            v2 q = { state.pos.x - wall.a.x, state.pos.y - wall.a.y };

            f32 denominator = dir.x * p.x - dir.y * p.y;

            if (fabsf(denominator) < 0.000000001f) continue;

            f32 t = (p.x * q.y - p.y * q.x) / denominator;
            f32 u = (dir.x * q.y - dir.y * q.x) / denominator;
            if (t >= 0 && u >= 0 && u <= 1) {
                v2 intersection = { state.pos.x + t * dir.x, state.pos.y + t * dir.y };
                v2 dist_vec = (v2) { intersection.x - state.pos.x, intersection.y - state.pos.y };
                f32 dist = length(dist_vec);
                if(dist < min_distance) {
                    min_distance = dist;
                    min_wall = &wall;
                    min_intersection = &intersection;
                }

            }
        }
        if (min_wall != NULL) {
            hit.val = 1;
            hit.pos = *min_intersection;
        }



        //hit.pos = (v2) { pos.x + sidedist.x, pos.y + sidedist.y };
        if (hit.pos.x != 0 && hit.pos.y != 0) {
                state.minimap.arr[x] = hit.pos;
        }

        u32 color;
        switch (hit.val) {
            //case 0: continue; break;
            case 1: color = 0xFF0000FF; break;
            case 2: color = 0xFF00FF00; break;
            case 3: color = 0xFFFF0000; break;
            case 4: color = 0xFFFF00FF; break;
        }

        // darken colors on y-sides
        if (hit.side == 1) {
            const u32
                br = ((color & 0xFF00FF) * 0xC0) >> 8,
                g  = ((color & 0x00FF00) * 0xC0) >> 8;

            color = 0xFF000000 | (br & 0xFF00FF) | (g & 0x00FF00);
        }


        // distance to hit
        const f32 dperp = min_distance;

        // perform perspective division, calculate line height relative to
        // screen center
        const int
            h = (int) (SCREEN_HEIGHT / dperp),
            y0 = max((SCREEN_HEIGHT / 2) - (h / 2), 0),
            y1 = min((SCREEN_HEIGHT / 2) + (h / 2), SCREEN_HEIGHT - 1);
        verline(x, 0, y0, 0xFF202020);
        verline(x, y0, y1, color);
        verline(x, y1, SCREEN_HEIGHT - 1, 0xFF505050);
    }
}

static void rotate(f32 rot) {
    const v2 d = state.dir, p = state.plane;
    state.dir.x = d.x * cos(rot) - d.y * sin(rot);
    state.dir.y = d.x * sin(rot) + d.y * cos(rot);
    state.plane.x = p.x * cos(rot) - p.y * sin(rot);
    state.plane.y = p.x * sin(rot) + p.y * cos(rot);
}


void load_map(char* path) {

    FILE* input = fopen(path, "r");
    ASSERT(input, "Cannot find Map!");

    enum { SCAN_SECTOR, SCAN_WALL, SCAN_NONE } scan = SCAN_NONE;
    char line[1024], buf[64];
    while (fgets(line, sizeof(line), input)) {
        const char* p = line;
        while(isspace(*p)) {
            p++;
        }

        if (!*p || *p == '#') {
            continue;
        } else if (*p == '[') {
            strncpy(buf, p + 1, sizeof(buf));
            const char* section = strtok(buf, "]");
            if(!section) { fclose(input); ASSERT(false, "Incomplete Header"); };
            if (!strcmp(section, "SECTOR")) { scan = SCAN_SECTOR; }
            else if (!strcmp(section, "WALL")) { scan = SCAN_WALL; }
            else { fclose(input); ASSERT(false, "Unknown Header"); }
        } else {
            switch(scan) {
                case SCAN_WALL: {
                    Wall* wall = &state.walls.arr[state.walls.n++];
                    if (sscanf(p,
                                "%f %f %f %f %d",
                                &wall->a.x,
                                &wall->a.y,
                                &wall->b.x,
                                &wall->b.y,
                                &wall->portal) != 5) {
                        fclose(input);
                        ASSERT(false, "Incomplete Wall Definition");
                    }
                                }break;
                case SCAN_SECTOR: {
                    Sector* sector = &state.sectors.arr[state.sectors.n++];
                    if (sscanf(p,
                                "%lu %lu %lu %f %f",
                                &sector->id,
                                &sector->index,
                                &sector->nwalls,
                                &sector->floor,
                                &sector->ceiling) != 5) {
                        fclose(input);
                        ASSERT(false, "Incomplete Sector Definition");
                    }
                                  } break;
                default: fclose(input); ASSERT(false, "No Header Set");
            }
        }
    }
    /*printf("SECTORS:\n");
    for(int i = 0; i < state.sectors.n; i++){
        Sector s = state.sectors.arr[i];
        printf("\tID: %lu, INDEX: %lu, WALLS: %lu, CEIL: %.1f, FLOOR: %.1f\n", s.id, s.index, s.nwalls, s.ceiling, s.floor);
    }
    printf("WALLS:\n");
    for(int i = 0; i < state.walls.n; i++){
        Wall w = state.walls.arr[i];
        printf("\tA: (%.1f, %.1f), B: (%.1f, %.1f), PORTAL: %d\n", w.a.x, w.a.y, w.b.x, w.b.y, w.portal);
    }*/
    return;
}

const int MM_SCALE = 20;
void minimap(){
    f32 angle = atan(state.dir.y/state.dir.x);
    int x = state.pos.x * MM_SCALE;
    int y = state.pos.y * MM_SCALE;
    v2 a = rotate_vec((v2) { 0, 5}, angle);
    v2 b = rotate_vec((v2) { -5, -5}, angle);
    v2 c = rotate_vec((v2) { 5, -5}, angle);

    SDL_Vertex triangleVertex[3] =
    {
        {
            { a.x + x, a.y + y }, /* first point location */
            { 255, 0, 0, 0xFF }, /* first color */
            { 0.f, 0.f }
        },
        {
            { b.x + x, b.y + y }, /* second point location */
            { 0,255,0, 0xFF }, /* second color */
            { 0.f, 0.f }
        },
        {
            { c.x + x, c.y + y }, /* third point location */
            { 0,0,255, 0xFF }, /* third color */
            { 0.f, 0.f }
        }
    };

    SDL_SetRenderDrawColor(state.minimap.renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(state.minimap.renderer);

    SDL_SetRenderDrawColor(state.minimap.renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    for(int i = 0; i < state.walls.n; i++) {
        Wall wall = state.walls.arr[i];
        ASSERT(SDL_RenderDrawLine(state.minimap.renderer, wall.a.x * MM_SCALE, wall.a.y * MM_SCALE, wall.b.x*MM_SCALE, wall.b.y * MM_SCALE) == 0, "Cant Draw Wall!");
    }

    for (int i = 0; i < SCREEN_WIDTH; i++) {
        v2 ray = state.minimap.arr[i];
        ASSERT(SDL_RenderDrawLine(state.minimap.renderer, state.pos.x * MM_SCALE, state.pos.y* MM_SCALE, ray.x * MM_SCALE, ray.y * MM_SCALE) == 0, "Cant Draw Ray!");
    }
    bzero(state.minimap.arr, SCREEN_WIDTH);

    SDL_RenderGeometry(state.minimap.renderer, NULL, triangleVertex, 3, NULL, 0);
    SDL_RenderPresent(state.minimap.renderer);
}

int main(int argc, char *argv[]) {
    ASSERT(
        !SDL_Init(SDL_INIT_VIDEO),
        "SDL failed to initialize: %s\n",
        SDL_GetError());

    state.window =
        SDL_CreateWindow(
            "DEMO",
            SDL_WINDOWPOS_CENTERED_DISPLAY(0),
            SDL_WINDOWPOS_CENTERED_DISPLAY(0),
            1280,
            720,
            SDL_WINDOW_ALLOW_HIGHDPI);
    ASSERT(
        state.window,
        "failed to create main window: %s\n", SDL_GetError());

    state.renderer =
        SDL_CreateRenderer(state.window, -1, SDL_RENDERER_PRESENTVSYNC);
    ASSERT(
        state.renderer,
        "failed to create SDL renderer: %s\n", SDL_GetError());

    state.minimap.window =
        SDL_CreateWindow(
            "MINIMAP",
            SDL_WINDOWPOS_CENTERED_DISPLAY(0),
            SDL_WINDOWPOS_CENTERED_DISPLAY(0),
            1280/2,
            720/2,
            SDL_WINDOW_SHOWN);
    ASSERT(
        state.minimap.window,
        "failed to create minimap window: %s\n", SDL_GetError());

    state.minimap.renderer =
        SDL_CreateRenderer(state.minimap.window, -1, SDL_RENDERER_PRESENTVSYNC);
    ASSERT(
        state.minimap.renderer,
        "failed to create SDL renderer: %s\n", SDL_GetError());
    //exit(0);


    state.texture =
        SDL_CreateTexture(
            state.renderer,
            SDL_PIXELFORMAT_ABGR8888,
            SDL_TEXTUREACCESS_STREAMING,
            SCREEN_WIDTH,
            SCREEN_HEIGHT);
    ASSERT(
        state.texture,
        "failed to create SDL texture: %s\n", SDL_GetError());


    load_map("sector.txt");

    state.pos = (v2) { 2, 2 };
    state.dir = normalize(((v2) { -1.0f, 0.1f }));
    state.plane = (v2) { 0.0f, 0.66f };
    state.current_sector = 0;


    SDL_Surface* image = SDL_LoadBMP("gun.bmp");


    while (!state.quit) {
        bool left, right, up, down = false;
        printf("POS: %f, %f\n", state.pos.x, state.pos.y);
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    state.quit = true;
                    break;
            }
        }

        const f32 rotspeed = 2.0f * 0.016f;
        const f32 movespeed = 2.0f * 0.016f;

        const u8 *keystate = SDL_GetKeyboardState(NULL);
        if (keystate[SDL_SCANCODE_LEFT]) {
            rotate(+rotspeed);
        }

        if (keystate[SDL_SCANCODE_RIGHT]) {
            rotate(-rotspeed);
        }

        if (keystate[SDL_SCANCODE_UP]) {
            state.pos.x += state.dir.x * movespeed;
            state.pos.y += state.dir.y * movespeed;

        }

        if (keystate[SDL_SCANCODE_DOWN]) {
            state.pos.x -= state.dir.x * movespeed;
            state.pos.y -= state.dir.y * movespeed;
        }

        if (keystate[SDL_SCANCODE_ESCAPE]) {
            state.quit = true;
        }

        memset(state.pixels, 0, sizeof(state.pixels));
        render();
        minimap();

        SDL_UpdateTexture(state.texture, NULL, state.pixels, SCREEN_WIDTH * 4);
        SDL_RenderCopyEx(
            state.renderer,
            state.texture,
            NULL,
            NULL,
            0.0,
            NULL,
            SDL_FLIP_VERTICAL);
        SDL_RenderPresent(state.renderer);
    }
    SDL_DestroyTexture(state.texture);
    SDL_DestroyRenderer(state.renderer);
    SDL_DestroyWindow(state.window);
    return 0;
}


