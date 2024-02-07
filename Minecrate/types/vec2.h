#ifndef VEC2_H
#define VEC2_H

#include <stdint.h>

// 8 bit signed vector3
typedef struct {
	int8_t x;
	int8_t y;
} vec2i8_t;

_Bool vec2i8_t_equals(vec2i8_t a, vec2i8_t b);

// 16 bit signed vector3
typedef struct {
	int16_t x;
	int16_t y;
} vec2i16_t;

_Bool vec2i16_t_equals(vec2i16_t a, vec2i16_t b);

// 32 bit signed vector3
typedef struct {
	int32_t x;
	int32_t y;
} vec2i32_t;

_Bool vec2i32_t_equals(vec2i32_t a, vec2i32_t b);

// 8 bit unsigned vector3
typedef struct {
	uint8_t x;
	uint8_t y;
} vec2u8_t;

_Bool vec2u8_t_equals(vec2u8_t a, vec2u8_t b);

// 16 bit unsigned vector3
typedef struct {
	uint16_t x;
	uint16_t y;
} vec2u16_t;

_Bool vec2u16_t_equals(vec2u16_t a, vec2u16_t b);

// 32 bit unsigned vector3
typedef struct {
	uint32_t x;
	uint32_t y;
} vec2u32_t;

_Bool vec2u32_t_equals(vec2u32_t a, vec2u32_t b);

#endif