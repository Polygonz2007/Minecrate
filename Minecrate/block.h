#ifndef BLOCK_H
#define BLOCK_H

#include <stdint.h>

// DEFINE BLOCK STRUCT
typedef struct {
	uint8_t type; // max 254 types of blocks for now, excluding air and undefined. 0 is undefined, and air is 1
} block_t;

// Struct constructor
block_t block_t_new(uint8_t type);
block_t block_t_new_from_name(char type[]);

uint16_t num_block_types;

// DEFINE BLOCK NAMES FOR EACH TYPE
static const char block_names[8][16] = {
	"Undefined",
	"Air",
	"Bedrock",
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
	BLOCK_GRASS,
	BLOCK_DIRT,
	BLOCK_STONE,
	BLOCK_COBBLESTONE,
	BLOCK_SAND
};

// DEFINE BLOCK HARDNESS(ES)
static const uint8_t block_hardness[8] = { // max block hardness can be 255, and lowest 0. (0 cannot be broken)
	0,	// UNDEFINED
	0,	// AIR
	0,	// BEDROCK
	3,	// GRASS
	2,	// DIRT
	5,	// STONE
	5,	// COBBLESTONE
	2	// SAND
};

#endif