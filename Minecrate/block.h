#ifndef BLOCK_H
#define BLOCK_H

#include <stdint.h>

// DEFINE BLOCK STRUCT
typedef struct {
	uint8_t type; // max 255 types of blocks for now, excluding air
} block_t;


// DEFINE BLOCK NAMES FOR EACH TYPE
const char block_names[4][8] = {
  "Air",
  "Grass",
  "Dirt",
  "Sand"
};

// Air----|Grass--|Dirt---|Sand---|

// DEFINE BLOCK HARDNESS(ES)
const uint8_t block_hardness[4] = { // max block hardness can be 255, and lowest 0.
	0,	// AIR
	3,	// GRASS
	2,	// DIRT
	2	// SAND
};

#endif