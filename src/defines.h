#ifndef DEFINES_H
#define DEFINES_H

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long long i64;

typedef float real32;
typedef double real64;

static_assert(sizeof(u8) == 1, "u8 wrong size.");
static_assert(sizeof(u16) == 2, "u16 wrong size.");
static_assert(sizeof(u32) == 4, "u32 wrong size.");
static_assert(sizeof(u64) == 8, "u64 wrong size.");

static_assert(sizeof(i8) == 1, "i8 wrong size.");
static_assert(sizeof(i16) == 2, "i16 wrong size.");
static_assert(sizeof(i32) == 4, "i32 wrong size.");
static_assert(sizeof(i64) == 8, "i64 wrong size.");

static_assert(sizeof(real32) == 4, "float wrong size.");
static_assert(sizeof(real64) == 8, "double wrong size.");

#endif