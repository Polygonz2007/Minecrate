
#include <malloc.h>
#include "perlinNoise.h"

#include "vec2.h"
#include "vec3.h"

#include "chunk.h"
#include "block.h"

// CHUNK DATA
static block_t *chunk_data;		// Stores every block type, at every X, Y, and Z position, for every chunk.
static vec2i16_t *chunk_locs;	// Stores position of chunks in "chunk_data". Allows for 2 million chunks in each direction, including negative.
static uint8_t *chunk_status;	// 0: Ready to be used, 1: loading, 2: idle
static uint8_t *chunk_buffer;	// Used for storing 2d heights when calculating blocks within chunk

static uint8_t sea_level = 63; // temporary


// Memory functions
int init_chunks() {
	if (render_distance > 64) // Make sure renderdistance isn't too big or too teeny
		render_distance = 64;
	if (render_distance < 4)
		render_distance = 4;

	num_chunks = (render_distance * 2 + 1); // (rd + 1 + rd), bc both directions + current chunk
	num_chunks *= num_chunks; // Squared, bc 2 dimensions, ofc idiot

	chunk_data_size = chunk_size.x * chunk_size.y * chunk_size.z;

	// Allocate memory for chunk data
	chunk_data = malloc(chunk_data_size * num_chunks * sizeof(block_t));
	chunk_locs = malloc(num_chunks * sizeof(vec3i16_t));
	chunk_status = malloc(num_chunks * sizeof(uint8_t));
	chunk_buffer = malloc(chunk_size.x * chunk_size.z * sizeof(uint8_t));
}

int free_chunks() { // WARNING: any chunk functions including load_mesh or load_chunk should NOT be caled after this is called.
	// Free all allocated memory
	printf("\nChunk data!");
	free(chunk_data);
	printf("\nLOCS!");
	free(chunk_locs);
	printf("\nA-la status!");
	free(chunk_status);
	printf("\nLast but least (sorry buffer) buffer!");
	free(chunk_buffer);
	printf("\nIt worked and somehow idfk how did WHAT:!??!?!");
}


// Generation
int load_chunk(vec2i16_t chunk_pos) {
	int32_t chunk_index = get_chunk_index(chunk_pos);

	if (chunk_index == -1)
		return -1; // error, the chunk already exist

	// it fine we load chunke
	chunk_status[chunk_index] = 1; // loading

	// Calculate terrain heights, and store to buffer
	int32_t global_cx = chunk_pos.x * 16;
	int32_t global_cy = chunk_pos.y * 16;

	for (uint16_t x = 0; x < chunk_size.x; ++x) {
		for (uint16_t y = 0; y < chunk_size.z; ++y) {
			int32_t cx = global_cx + x;
			int32_t cy = global_cy + y;

			chunk_buffer[cx + (cy * chunk_size.x)] = sea_level + -2 + (uint8_t)(32.0f * sample_perlin_octaves(
				cx / 48.0f,                     // X
				cy / 48.0f,                     // Y
				5,                              // OCTAVES
				2.0f,                           // LACUNARITY
				0.47f)                          // PERSISTANCE
				- sample_perlin(cx / 154.0f, cy / 154.0f) * 60.0f);
		}
	}

	// fill inn with depths, 1: grass 4: dirt, rest stone for now then bedrock at bedrock
	// also: 0 in the noise is sea level, (make variable) but should be 63 so when ur standing on shore ur at 64 :D
	for (uint16_t x = 0; x < chunk_size.x; ++x) {
		for (uint16_t y = 0; y < chunk_size.y; ++y) {
			for (uint16_t z = 0; z < chunk_size.z; ++z) {
				int32_t i = get_block_index((vec3i32_t) { global_cx + x, y, global_cy + z });

				// Valeed?
				if (i == -1)
					return -1; // failed

				// Si
				uint8_t cb = 1; // air
				uint16_t h = chunk_buffer[x + (y * chunk_size.x)];

				if (y > h)
					cb = 2; // grass

				chunk_data[i] = block_t_new(cb);
			}
		}
	}

	// SO here we add the chunk location MAYBE THAT IS A GOOD IDEA?!??!?!??!!??!? MY DAYSSS PAST POLY WHY U SO DUM
	chunk_locs[chunk_index] = (vec2i16_t) { chunk_pos.x, chunk_pos.y };
	chunk_status[chunk_index] = 0; /// good YEYSYSYYSYS 

	return 0;
}

int unload_chunk(vec2i16_t chunk_pos) {
	return 0;
}


// Block functions
int32_t get_chunk_index_data(vec2i16_t chunk_pos) {
	for (uint16_t i = 0; i < num_chunks; ++i) {
		if (vec2i16_t_equals(chunk_pos, chunk_locs[i])) {
			return i * chunk_data_size;
		}
	}

	return -1; // did not find a chunk
}

int32_t get_chunk_index(vec2i16_t chunk_pos) {
	for (uint16_t i = 0; i < num_chunks; ++i) {
		if (vec2i16_t_equals(chunk_pos, chunk_locs[i])) {
			return i;
		}
	}

	return -1; // did not find a chunk
}

int32_t get_block_index(vec3i32_t block_pos) {
	vec2i16_t chunk_pos = get_chunk_pos(block_pos);
	int32_t chunk_index = get_chunk_index_data(chunk_pos);

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