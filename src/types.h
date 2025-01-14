#ifndef TYPES_H
#define TYPES_H

#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define ASSERT(_e, ...) if (!(_e)) { fprintf(stderr, __VA_ARGS__); exit(0); }




typedef float    f32;
typedef double   f64;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef size_t   usize;
typedef ssize_t  isize;

#define SCREEN_WIDTH 384
#define SCREEN_HEIGHT 216

typedef struct v2_s { f32 x, y; } v2;
typedef struct v2i_s { i32 x, y; } v2i;
typedef struct rect { v2 a, b, c, d; } rect;

#define dot(v0, v1)                  \
    ({ const v2 _v0 = (v0), _v1 = (v1); (_v0.x * _v1.x) + (_v0.y * _v1.y); })
#define sub(v0, v1) ( (v2) { v0.x - v1.x, v0.y - v1.y } )

v2 rotate_vec(v2 v, f32 deg) {
    return (v2) { v.x * cos(deg) - v.y * sin(deg), v.x * sin(deg) + v.y * cos(deg) };
}

#define length(v) ({ const v2 _v = (v); sqrtf(dot(_v, _v)); })
#define normalize(u) ({              \
        const v2 _u = (u);           \
        const f32 l = length(_u);    \
        (v2) { _u.x / l, _u.y / l }; \
    })
#define min(a, b) ({ __typeof__(a) _a = (a), _b = (b); _a < _b ? _a : _b; })
#define max(a, b) ({ __typeof__(a) _a = (a), _b = (b); _a > _b ? _a : _b; })
#define sign(a) ({                                       \
        __typeof__(a) _a = (a);                          \
        (__typeof__(a))(_a < 0 ? -1 : (_a > 0 ? 1 : 0)); \
    })


typedef struct {
    usize id, index, nwalls;
    f32 floor, ceiling;
} Sector;

typedef struct {
    v2 a, b;
    u32 portal;
} Wall;
typedef struct {
    SDL_Window *window;
    SDL_Texture *texture;
    SDL_Renderer *renderer;
    u32 pixels[SCREEN_WIDTH * SCREEN_HEIGHT];
    struct { Sector arr[32]; usize n; } sectors;
    struct { Wall arr[128]; usize n; } walls;
    bool quit;

    struct { SDL_Window *window; SDL_Renderer *renderer; v2 arr[SCREEN_WIDTH]; } minimap;

    usize current_sector;

    v2 pos, dir, plane;
} State;


rect create_rect(v2 a, v2 b) {
    const v2 ab = sub(b, a);
    const v2 c = rotate_vec(ab, 90);
    const v2 bc = sub(c, a);
    const v2 d = rotate_vec(bc, 90);
    return (rect) { a, b, c, d };
}

bool point_in_rect(rect rect, v2 point) {

}

#endif // !TYPES_H
