#ifndef RDIYE_LIB_H
#define RDIYE_LIB_H

#pragma once

#include "math.h"

#define INTERNAL static
#define LOCAL static
#define GLOBAL static

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long long i64;

typedef float f32;
typedef double f64;

typedef unsigned char b8;
typedef unsigned int b32;

static_assert(sizeof(u8) == 1, "u8 size error");
static_assert(sizeof(u16) == 2, "u16 size error");
static_assert(sizeof(u32) == 4, "u32 size error");
static_assert(sizeof(u64) == 8, "u64 size error");

static_assert(sizeof(i8) == 1, "i8 size error");
static_assert(sizeof(i16) == 2, "i16 size error");
static_assert(sizeof(i32) == 4, "i32 size error");
static_assert(sizeof(i64) == 8, "i64 size error");

static_assert(sizeof(f32) == 4, "f32 size error");
static_assert(sizeof(f64) == 8, "f64 size error");

static_assert(sizeof(b8) == 1, "b8 size error");
static_assert(sizeof(b32) == 4, "b32 size error");

#if 0
#if DEBUG_BUILD
#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#else
#define Assert(Expression)
#endif
#else
#if DEBUG_BUILD
#define Assert(Expression) if(!(Expression)) {printf("assert fired: %s, line %d\n", __FILE__, __LINE__);}
#else
#define Assert(Expression)
#endif
#endif

#define Kilobytes(value) ((value) * 1024LL)
#define Megabytes(value) (Kilobytes(value) * 1024LL)
#define Gigabytes(value) (Megabytes(value) * 1024LL)
#define Terabytes(value) (Gigabytes(value) * 1024LL)

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))
#define Minimum(A, B) ((A < B) ? (A) : (B))
#define Maximum(A, B) ((A > B) ? (A) : (B))

#define PI32 3.14159265359f

#define DegreesToRadians(angle) ((angle) * PI32 / 180)

inline f32 Square(f32 a)
{
    f32 result = a * a;

    return(result);
}

inline f32 Cosine(f32 theta)
{
    f32 result = cosf(theta);

    return(result);
}

inline f32 Sine(f32 theta)
{
    f32 result = sinf(theta);

    return(result);
}

union vec2
{
    struct
    {
        f32 x, y;
    };
    f32 e[2];
};

union vec3
{
    struct
    {
        f32 x, y, z;
    };
    struct
    {
        f32 r, g, b;
    };
    struct
    {
        vec2 xy;
        f32 ignored_;
    };
    f32 e[3];
};

union vec4
{
    struct
    {
        f32 x, y, z, w;
    };
    struct
    {
        f32 r, g, b, a;
    };
    struct
    {
        vec3 xyz;
        f32 ignored_;
    };
    f32 e[4];
};

inline vec2 Vec2(f32 x, f32 y)
{
    vec2 result;

    result.x = x;
    result.y = y;

    return(result);
}

inline vec3 Vec3(f32 x, f32 y, f32 z)
{
    vec3 result;

    result.x = x;
    result.y = y;
    result.z = z;

    return(result);
}

inline vec3 Vec3(vec2 xy, f32 z)
{
    vec3 result;

    result.x = xy.x;
    result.y = xy.y;
    result.z = z;

    return(result);
}

inline vec4 Vec4(f32 x, f32 y, f32 z, f32 w)
{
    vec4 result;

    result.x = x;
    result.y = y;
    result.z = z;
    result.w = w;

    return(result);
}

inline vec4 Vec4(vec3 xyz, f32 w)
{
    vec4 result;

    result.x = xyz.x;
    result.y = xyz.y;
    result.z = xyz.z;
    result.w = w;

    return(result);
}

inline vec2 operator*(f32 a, vec2 b)
{
    vec2 result;
    
    result.x = a * b.x;
    result.y = a * b.y;

    return(result);
}

inline vec2 operator*(vec2 b, f32 a)
{
    vec2 result = a * b;

    return(result);
}

inline vec2 &operator*=(vec2 &b, f32 a)
{
    b = a * b;

    return(b);
}

inline vec2 operator-(vec2 a)
{
    vec2 result;

    result.x = -a.x;
    result.y = -a.y;

    return(result);
}

inline vec2 operator+(vec2 a, vec2 b)
{
    vec2 result;

    result.x = a.x + b.x;
    result.y = a.y + b.y;

    return(result);
}

inline vec2 &operator+=(vec2 &a, vec2 b)
{
    a = a + b;

    return(a);
}

inline vec2 operator-(vec2 a, vec2 b)
{
    vec2 result;

    result.x = a.x - b.x;
    result.y = a.y - b.y;

    return(result);
}

inline vec2 Hadamard(vec2 a, vec2 b)
{
    vec2 result = {a.x * b.x, a.y * b.y};

    return(result);
}

inline f32 DotProduct(vec2 a, vec2 b)
{
    f32 result = a.x * b.x + a.y * b.y;

    return(result);
}

inline f32 LengthSquared(vec2 a)
{
    f32 result = DotProduct(a, a);

    return(result);
}

inline f32 Length(vec2 a)
{
    f32 result = sqrtf(LengthSquared(a));

    return(result);
}

inline vec3 operator*(f32 a, vec3 b)
{
    vec3 result;
    
    result.x = a * b.x;
    result.y = a * b.y;
    result.z = a * b.z;

    return(result);
}

inline vec3 operator*(vec3 b, f32 a)
{
    vec3 result = a * b;

    return(result);
}

inline vec3 &operator*=(vec3 &b, f32 a)
{
    b = a * b;

    return(b);
}

inline vec3 operator/(vec3 a, f32 b)
{
    vec3 result;
    
    result.x = a.x / b;
    result.y = a.y / b;
    result.z = a.z / b;

    return(result);
}

inline vec3 &operator/=(vec3 &b, f32 a)
{
    b = b / a;

    return(b);
}

inline vec3 operator-(vec3 a)
{
    vec3 result;

    result.x = -a.x;
    result.y = -a.y;
    result.z = -a.z;

    return(result);
}

inline vec3 operator+(vec3 a, vec3 b)
{
    vec3 result;

    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;

    return(result);
}

inline vec3 &operator+=(vec3 &a, vec3 b)
{
    a = a + b;

    return(a);
}

inline vec3 operator-(vec3 a, vec3 b)
{
    vec3 result;

    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;

    return(result);
}

inline vec3 &operator-=(vec3 &a, vec3 b)
{
    a = a - b;

    return(a);
}

inline vec3 Hadamard(vec3 a, vec3 b)
{
    vec3 result = {a.x * b.x, a.y * b.y, a.z * b.z};

    return(result);
}

inline f32 DotProduct(vec3 a, vec3 b)
{
    f32 result = a.x * b.x + a.y * b.y + a.z * b.z;

    return(result);
}

inline f32 LengthSquared(vec3 a)
{
    f32 result = DotProduct(a, a);

    return(result);
}

inline f32 Length(vec3 a)
{
    f32 result = sqrtf(LengthSquared(a));

    return(result);
}

inline vec3 Normalize(vec3 a)
{
    f32 mag = Length(a);
    vec3 result = a / mag;

    return(result);
}

inline vec3 Cross(vec3 a, vec3 b)
{
    vec3 result = {};
    result.x = a.y * b.z - a.z * b.y;
    result.y = a.x * b.z - a.z * b.x;
    result.z = a.x * b.y - a.y * b.x;

    return(result);
}

inline f32 DotProduct(vec4 a, vec4 b)
{
    f32 result = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;

    return(result);
}

struct rect3
{
    vec3 min;
    vec3 max;
};

inline rect3 Rect3(vec3 min, vec3 max)
{
    rect3 result;

    result.min = min;
    result.max = max;

    return(result);
}

inline b32 IsInsideRectangle(rect3 rect, vec3 vec)
{
    b32 result = ((vec.x >= rect.min.x) &&
                  (vec.y >= rect.min.y) &&
                  (vec.z >= rect.min.z) &&
                  (vec.x < rect.max.x) &&
                  (vec.y < rect.max.y) &&
                  (vec.z < rect.max.z));

    return(result);
}

struct mat4x4
{
    f32 e[4][4];
};

inline mat4x4 operator*(mat4x4 A, mat4x4 B)
{
    mat4x4 C = {{
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    }};;
    for(i32 i = 0; i < 4; i++)
    {
        for(i32 j = 0; j < 4; j++)
        {
            for(i32 k = 0; k < 4; k++)
            {
                C.e[i][j] += A.e[i][k] * B.e[k][j];
            }
        }
    }

    return(C);
}

inline vec3 operator*(mat4x4 A, vec4 b)
{
    vec3 result = {};
    for(i32 i = 0; i < 4; i++)
    {
        result.e[i] = DotProduct(Vec4(A.e[i][0], A.e[i][1], A.e[i][2], A.e[i][3]), b);
    }
    
    return(result);
}

inline vec3 operator*(mat4x4 A, vec3 b)
{
    vec4 c = Vec4(b, 1);
    vec3 result = A * c;
    
    return(result);
}

mat4x4 Identity(void)
{
    mat4x4 result = {{
        {1, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 1}
    }};

    return(result);
}

mat4x4 Translation(f32 x, f32 y, f32 z)
{
    mat4x4 result = {{
        {1, 0, 0, x},
        {0, 1, 0, y},
        {0, 0, 1, z},
        {0, 0, 0, 1}
    }};

    return(result);
}

mat4x4 Translation(vec3 v)
{
    mat4x4 result = Translation(v.x, v.y, v.z);

    return(result);
}

mat4x4 RotationX(f32 phi)
{
    f32 cos = Cosine(phi);
    f32 sin = Sine(phi);
    mat4x4 result = {{
        {1, 0, 0, 0},
        {0, cos, -sin, 0},
        {0, sin, cos, 0},
        {0, 0, 0, 1}
    }};

    return(result);
}

mat4x4 RotationY(f32 phi)
{
    f32 cos = Cosine(phi);
    f32 sin = Sine(phi);
    mat4x4 result = {{
        {cos, 0, sin, 0},
        {0, 1, 0, 0},
        {-sin, 0, cos, 0},
        {0, 0, 0, 1}
    }};

    return(result);
}

mat4x4 RotationZ(f32 phi)
{
    f32 cos = Cosine(phi);
    f32 sin = Sine(phi);
    mat4x4 result = {{
        {cos, -sin, 0, 0},
        {sin, cos, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 1}
    }};

    return(result);
}

mat4x4 RotationP(mat4x4 rotation, vec3 point)
{
    mat4x4 Tp = Translation(point);
    mat4x4 T_minus_p = Translation(-point);
    mat4x4 result = Tp * rotation * T_minus_p;

    return(result);
}

mat4x4 Scaling(f32 x, f32 y, f32 z)
{
   mat4x4 result = {{
        {x, 0, 0, 0},
        {0, y, 0, 0},
        {0, 0, z, 0},
        {0, 0, 0, 1}
    }};

    return(result); 
}

mat4x4 Shear(i32 i, i32 j, f32 s)
{
    Assert(i < 4);
    Assert(j < 4);
    mat4x4 result = Identity();
    result.e[i][j] = s;

    return(result); 
}

mat4x4 Orthographic(f32 left, f32 bottom, f32 near_plane, f32 right, f32 top, f32 far_plane)
{
    // NOTE: we are looking at the negative z axis so technically we should have near_plane > far_plane
    // however it is more intuitive to think of the near plane has having a lower z value
    near_plane = -near_plane;
    far_plane = -far_plane;
    Assert(near_plane > far_plane);

    mat4x4 S = Scaling((2 / (right - left)),
                       (2 / (top - bottom)),
                       (2 / (far_plane - near_plane)));
    mat4x4 T = Translation((-(left + right) / 2),
                           (-(top + bottom) / 2),
                           (-(far_plane + near_plane) / 2));
    mat4x4 result = S * T;

    return(result); 
}

mat4x4 Orthographic(vec3 min_corner, vec3 max_corner)
{
    mat4x4 result = Orthographic(min_corner.x, min_corner.y, min_corner.z, max_corner.x, max_corner.y, max_corner.z);

    return(result);
}

mat4x4 Orthographic(rect3 rect)
{
    mat4x4 result = Orthographic(rect.min, rect.max);

    return(result);
}


// TODO: remove this later
#include "lib/glm/glm.hpp"
glm::vec3 Vec3ToGlm(vec3 src)
{
    glm::vec3 result = glm::vec3(src.x, src.y, src.z);

    return(result);
}
vec3 GlmToVec3(glm::vec3 vec)
{
    vec3 result = Vec3(vec.x, vec.y, vec.z);

    return(result);
}
glm::mat4 Mat4ToGlm(mat4x4 src)
{
    glm::mat4 result = glm::mat4(src.e[0][0], src.e[1][0], src.e[2][0], src.e[3][0],
                                src.e[0][1], src.e[1][1], src.e[2][1], src.e[3][1],
                                src.e[0][2], src.e[1][2], src.e[2][2], src.e[3][2],
                                src.e[0][3], src.e[1][3], src.e[2][3], src.e[3][3]);

    return(result);
}

#endif