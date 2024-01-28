#ifndef VEC3_H
#define VEC3_H

#include <stdint.h>

// 8 bit signed vector3
typedef struct {
	int8_t x;
	int8_t y;
	int8_t z;
} vec3i8_t;

_Bool vec3i8_t_equals(vec3i8_t a, vec3i8_t b);

// 16 bit signed vector3
typedef struct {
	int16_t x;
	int16_t y;
	int16_t z;
} vec3i16_t;

_Bool vec3i16_t_equals(vec3i16_t a, vec3i16_t b);

// 32 bit signed vector3
typedef struct {
	int32_t x;
	int32_t y;
	int32_t z;
} vec3i32_t;

_Bool vec3i32_t_equals(vec3i32_t a, vec3i32_t b);

// 8 bit unsigned vector3
typedef struct {
	uint8_t x;
	uint8_t y;
	uint8_t z;
} vec3u8_t;

_Bool vec3u8_t_equals(vec3u8_t a, vec3u8_t b);

// 16 bit unsigned vector3
typedef struct {
	uint16_t x;
	uint16_t y;
	uint16_t z;
} vec3u16_t;

_Bool vec3u16_t_equals(vec3u16_t a, vec3u16_t b);

// 32 bit unsigned vector3
typedef struct {
	uint32_t x;
	uint32_t y;
	uint32_t z;
} vec3u32_t;

_Bool vec3u32_t_equals(vec3u32_t a, vec3u32_t b);

#endif