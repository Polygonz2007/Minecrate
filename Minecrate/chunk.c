
#include <stdio.h>
#include "chunk.h"
#include "block.h"
#include "vec3.h"
#include "vec2.h"

// Memory functions
int init_chunks() {
	if (render_distance > 64)
		render_distance = 64;
	if (render_distance < 4)
		render_distance = 4;

	num_chunks = (render_distance * 2 + 1);
	num_chunks *= num_chunks; // Squared, bc 2 dimensions, ofc idiot

	chunk_data_size = chunk_size.x * chunk_size.y * chunk_size.z;

	// Allocate memory for chunk data
	chunk_data = malloc(chunk_data_size * num_chunks * sizeof(block_t));
	chunk_locs = malloc(num_chunks * sizeof(vec3i16_t));
	chunk_status = malloc(num_chunks * sizeof(uint8_t));
}

int free_chunks() { // WARNING: any chunk functions including load_mesh or load_chunk should NOT be caled after this is called.
	// Free all allocated memory
	free(chunk_data);
	free(chunk_locs);
	free(chunk_status);
}


// Generation
int load_chunk(vec2i16_t chunk_pos) {
	int32_t chunk_index = get_chunk_index(chunk_pos);

	if (chunk_index == -1)
		return 1; // error

	// 2d array of heights

	// fill inn with depths, 1: grass 4: dirt, rest stone for now then bedrock at bedrock
	// also: 0 in the noise is sea level, (make variable) but should be 63 so when ur standing on shore ur at 64 :D
}

int unload_chunk(vec2i16_t chunk_pos) {

}


// Block functions
int32_t get_chunk_index(vec2i16_t chunk_pos) {
	for (uint16_t i = 0; i < num_chunks; ++i) {
		if (vec2i16_t_equals(chunk_pos, chunk_locs[i])) {
			return i * chunk_data_size;
		}
	}

	return -1; // did not find a chunk
}

int32_t get_block_index(vec3i32_t block_pos) {
	vec2i16_t chunk_pos = get_chunk_pos(block_pos);
	int32_t chunk_index = get_chunk_index(chunk_pos);

	if (chunk_index == -1)
		return -1; // Chunk does not exist, so no block

	// Within chunk
	vec3i16_t c_block_pos = get_block_in_chunk_pos(block_pos);

	// Find index of block_t
	int32_t block_index = chunk_index + block_pos.x + (chunk_size.x * block_pos.y) + (chunk_size.x * chunk_size.y * block_pos.z);

	// Return index
	return block_index;
}

block_t get_block(vec3i32_t block_pos) {
	int32_t block_index = get_block_index(block_pos);

	if (block_index == -1)
		return block_t_new(0); // no block so undefined
	
	block_t block = chunk_data[block_index];
	return block;
}


// UTIL
vec2i16_t get_chunk_pos(vec3i32_t block_pos) {
	return (vec2i16_t) { floor(block_pos.x / chunk_size.x), floor(block_pos.z / chunk_size.z) };
}

vec3i16_t get_block_in_chunk_pos(vec3i32_t block_pos) {
	vec3i16_t c_block_pos = { 0 };

	c_block_pos.x = block_pos.x % chunk_size.x;
	c_block_pos.y = block_pos.y;
	c_block_pos.z = block_pos.z % chunk_size.z;

	return c_block_pos;
}