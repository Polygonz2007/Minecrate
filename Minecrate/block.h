#ifndef BLOCK_H
#define BLOCK_H

#include <stdint.h>
#include <raylib.h>

// DEFINE BLOCK STRUCT
typedef struct {
	uint8_t type; // max 254 types of blocks for now, excluding air and undefined. 0 is undefined, and air is 1
} block_t;

// Struct constructor
block_t block_t_new(uint8_t type);
uint16_t num_block_types;

// DEFINE BLOCK NAMES FOR EACH TYPE
static const char block_names[9][16] = {
	"Undefined",
	"Air",
	"Bedrock",
	"Water",
	"Grass",
	"Dirt",
	"Stone",
	"Cobblestone",
	"Sand"
};

static enum block_type {
	BLOCK_UNDEFINED,
	BLOCK_AIR,
	BLOCK_BEDROCK,
	BLOCK_WATER,
	BLOCK_GRASS,
	BLOCK_DIRT,
	BLOCK_STONE,
	BLOCK_COBBLESTONE,
	BLOCK_SAND
};

// DEFINE BLOCK HARDNESS(ES)
static const uint8_t block_hardness[9] = { // max block hardness can be 255, and lowest 0. (0 cannot be broken)
	0,	// UNDEFINED
	0,	// AIR
	0,	// BEDROCK
	0,	// WATER
	3,	// GRASS
	2,	// DIRT
	5,	// STONE
	5,	// COBBLESTONE
	2	// SAND
};

static const Color block_colors[9] = {
	{ 255, 0, 0, 255 },
	{ 255, 255, 255, 0 },
	{ 0, 0, 0, 255 },
	{ 0, 121, 241, 63 },
	{ 60, 200, 30, 255 },
	{ 180, 127, 0, 255 },
	{ 180, 180, 180, 255 },
	{ 120, 120, 120, 255 },
	{ 240, 220, 170, 255 }
};

#endif