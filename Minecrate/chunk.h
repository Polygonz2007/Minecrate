#ifndef CHUNK_H
#define CHUNK_H

#include "block.h"
#include "vec3.h"

// CHUNK SETTINGS (cannot change at runtime! (yet))
uint16_t num_chunks;			// Total number of chunks avaliable in chunk_data, chunk_locs and chunk_status
uint16_t chunk_bytes;			// Total amount of bytes each chunk occupies in the "chunk_data" array.

const uint8_t renderdistance = 8;	// Chunks in each direction.
									// For example if this is 8 you have (8 + 1 + 8) chunks in each axis.

const vec3u16_t chunk_size = {		// Size of each chunk, in blocks.
	16,		// X
	256,	// Y
	16		// Z
};

// CHUNK DATA
block_t chunk_data;		// Stores every block type, at every X, Y, and Z position, for every chunk.
vec3i16_t chunk_locs;	// Stores position of chunks in "chunk_data". Allows for 2 million chunks in each direction, including negative.
uint8_t chunk_status;	// 0: Ready to be used, 1: loading, 2: idle somehow idk


// CHUNK FUNCTION PROTOTYPES
int init_chunks();		// Allocates both data arrays. Should be called before use of either array.
int free_chunks();		// Deallocates both data arrays. Should be called before exiting the program.

int32_t get_chunk_index(vec3i16_t chunk_pos);			// Returns its index in "chunk_data" array where the requested chunk starts. -1 if failed.
block_t get_block(uint32_t x, uint16_t y, uint32_t z);	// Returns block type for this GLOBAL location, automatically finds chunk. Returns UNDERFINED if failed

#endif