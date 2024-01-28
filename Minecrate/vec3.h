#ifndef VEC3_H
#define VEC3_H

#include <stdint.h>

// 8 bit signed vector3
typedef struct {
	int8_t x;
	int8_t y;
	int8_t z;
} vec3i8_t;

// 16 bit signed vector3
typedef struct {
	int8_t x;
	int8_t y;
	int8_t z;
} vec3i16_t;

// 32 bit signed vector3
typedef struct {
	int8_t x;
	int8_t y;
	int8_t z;
} vec3i32_t;



// 8 bit unsigned vector3
typedef struct {
	uint8_t x;
	uint8_t y;
	uint8_t z;
} vec3u8_t;

// 16 bit unsigned vector3
typedef struct {
	uint8_t x;
	uint8_t y;
	uint8_t z;
} vec3u16_t;

// 32 bit unsigned vector3
typedef struct {
	uint8_t x;
	uint8_t y;
	uint8_t z;
} vec3u32_t;

#endif