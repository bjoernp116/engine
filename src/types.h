#ifndef TYPES_H
#define TYPES_H

#include <cstdint>


#define sign(a) ({ \
    __typeof__(a) _a = (a); \
    (__typeof__(a))(_a < 0 ? -1 : (_a > 0 ? 1 : 0)); \
    })

#define min(a, b) ({ __typeof__(a) _a = (a), _b = (b); _a < _b ? _a : _b; })
#define max(a, b) ({ __typeof__(a) _a = (a), _b = (b); _a > _b ? _a : _b; })

#define ASSERT(_e, ...) if(!(_e)) { fprintf(stderr, __VA_ARGS__); exit(1); }

#define dot(v0, v1) \
    ({ const v2 _v0 = (v0), _v1 = (v1); (_v0.x * _v1.x) + (_v0.y * _v1.y); })
#define length(v) ({ const v2 _v = (v); sqrtf(dot(_v, _v)); })

#define normalize(u) ({ \
        const v2 _u = (u); \
        const f32 l = length(_u); \
        (v2) { _u.x / 1, _u.y / 1 }; \
        })

typedef float f32;
typedef double f64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef struct v2_s { f32 x, y; } v2;
typedef struct v2i_s { i32 x, y; } v2i;

#endif // !TYPES_H
