#ifndef CHUNK_H
#define CHUNK_H

#include <stdint.h>
#include "block.h"
#include "vec3.h"
#include "vec2.h"

// CHUNK SETTINGS (cannot change at runtime! (yet))
uint16_t num_chunks;			// Total number of chunks avaliable in chunk_data, chunk_locs and chunk_status
uint32_t chunk_data_size;		// Total amount of sizeof(block_t)'s each chunk occupies in the "chunk_data" array.
uint32_t chunk_mem_usage;

// Current position of player relative to chunks
static vec2i16_t current_chunk_pos = { 0, 0 };
static uint8_t sea_level = 63; // SEA LEVEL NOT RENDER DISTANCE STOP IT POLE

static uint8_t render_distance = 16;	// Chunks in each direction. (rd + 1 + rd) MIN 1 MAX 64 (so u dont kill comper)

static const vec3u16_t chunk_size = {	// Size of each chunk, in blocks.
	16,		// X
	256,	// Y
	16		// Z
};

static enum chunk_status {
	CHUNK_UNLOADED,
	CHUNK_LOADING,
	CHUNK_LOADED
};

// CHUNK FUNCTION PROTOTYPES
int init_chunks();		// Allocates both data arrays. Should be called before use of either array.
int free_chunks();		// Deallocates both data arrays. Should be called before exiting the program.

int load_chunk(vec2i16_t chunk_pos);	// Function for loading a chunk. Calculates all blocks within chunk and stores to chunk_data
int unload_chunk(vec2i16_t chunk_pos);	// Function for unloading a chunk. Sets chunk status to idle
 
int load_bounds(vec2i16_t pos);		// Loads unloaded chunks inside of bounds, defined by pos and renderdistance
int unload_bounds(vec2i16_t pos);	// Unloads chunks outside of bounds, defined by pos and renderdistance

int32_t get_chunk_index(vec2i16_t chunk_pos);	// Returns its index in "chunk_data" array where the requested chunk starts. -1 if failed.
int32_t get_block_index(vec3i16_t c_block_pos); // Get raw

block_t get_block(vec3i32_t block_pos);	// Returns block type for this GLOBAL location, automatically finds chunk. Returns UNDERFINED (0) if failed

// Utility!
vec2i16_t get_chunk_pos(vec3i32_t block_pos);	// Convert 3d block pos to 2d chunk pos
vec3i16_t get_block_in_chunk_pos(vec3i32_t block_pos);	// Get the local block pos
uint16_t get_total_loaded_chunks();

#endif