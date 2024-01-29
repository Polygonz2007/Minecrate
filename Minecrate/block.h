#ifndef BLOCK_H
#define BLOCK_H

#include <stdint.h>

// DEFINE BLOCK STRUCT
typedef struct {
	uint8_t type; // max 254 types of blocks for now, excluding air and undefined. 0 is undefined, and air is 1
} block_t;

// Struct constructor
block_t block_t_new(uint8_t type);


// DEFINE BLOCK NAMES FOR EACH TYPE
const char block_names[5][16] = {
	"Undefined",	// 0
	"Air",			// 1
	"Grass",		// 2
	"Dirt",			// 3
	"Sand"			// 4
};

// DEFINE BLOCK HARDNESS(ES)
const uint8_t block_hardness[5] = { // max block hardness can be 255, and lowest 0. (0 cannot be broken)
	0,	// UNDEFINED
	0,	// AIR
	3,	// GRASS
	2,	// DIRT
	2	// SAND
};

#endif