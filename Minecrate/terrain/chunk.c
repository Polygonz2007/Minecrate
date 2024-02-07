
#include <stdio.h>
#include <malloc.h>
#include <stdbool.h>

#include "perlinNoise.h"

#include "vec2.h"
#include "vec3.h"

#include "chunk.h"
#include "block.h"

// CHUNK DATA
static block_t *chunk_data;		// Stores every block type, at every X, Y, and Z position, for every chunk.
static vec2i16_t *chunk_locs;	// Stores position of chunks in "chunk_data". Allows for 2 million chunks in each direction, including negative.
static uint8_t *chunk_status;	// 0: Empty, 1: Loading, 2: Ready (blocks are filled and correct)
static int16_t *chunk_buffer;	// Used for storing 2d heights when calculating blocks within chunk


// Memory functions
int init_chunks() {
	// Make sure renderdistance isn't too big or too teeny
	if (render_distance > 64)
		render_distance = 64;
	if (render_distance < 1)
		render_distance = 1;

	// Calculate and do some stuff
	num_chunks = (render_distance * 2 + 1); // (rd + 1 + rd), bc both directions + current chunk
	num_chunks *= num_chunks;				// Squared, bc 2 dimensions, ofc idot

	chunk_data_size = chunk_size.x * chunk_size.y * chunk_size.z;
	printf("\n\n\nChunk data size: %d test x: %d", chunk_data_size, chunk_size.z);

	num_block_types = sizeof(block_names) / sizeof(block_names[0]);

	// Allocate memory for chunk data
	printf("\nAllocating memory for chunks...\n");

	chunk_data = malloc(chunk_data_size * num_chunks * sizeof(block_t));
	chunk_locs = malloc(num_chunks * sizeof(vec2i16_t));
	chunk_status = malloc(num_chunks * sizeof(uint8_t));
	chunk_buffer = malloc(chunk_size.x * chunk_size.z * sizeof(int16_t));

	// Track memory
	chunk_mem_usage = 0;
	chunk_mem_usage += chunk_data_size * num_chunks * sizeof(block_t);
	chunk_mem_usage += num_chunks * sizeof(vec2i16_t);
	chunk_mem_usage += num_chunks * sizeof(uint8_t);
	chunk_mem_usage += chunk_size.x * chunk_size.z * sizeof(int16_t);


	// Initialize all chunks as empty
	for (uint16_t i = 0; i < num_chunks; ++i) {
		chunk_status[i] = CHUNK_UNLOADED;
	}

	printf("Memory allocated.\n");
}

int free_chunks() { // WARNING: any chunk functions including load_mesh or load_chunk should NOT be caled after this is called.
	// Free all allocated memory
	printf("\n\nFreeing chunk memory...\n");
	free(chunk_data);
	free(chunk_locs);
	free(chunk_status);
	free(chunk_buffer);
	printf("Finished freeing chunks.\n");
}


// Generation
int load_chunk(vec2i16_t chunk_pos) {
	//printf("\n\nLoading chunk %d %d", chunk_pos.x, chunk_pos.y);
	int32_t chunk_index = get_chunk_index(chunk_pos);

	if (chunk_index != -1) {
		//printf("\nCant load chunk %d %d bc it already exists at index %d", chunk_pos.x, chunk_pos.y, chunk_index);
		return -1; // error, the chunk already exist
	} else {
		// Generate new index for use
		//printf("\nFinding next index...");
		chunk_index = next_chunk_index();
		//printf("\nLoadig chunk %d %d at index %d", chunk_pos.x, chunk_pos.y, chunk_index);
	}

	// it fine we load chunke
	chunk_status[chunk_index] = CHUNK_LOADING; // Loading
	chunk_locs[chunk_index] = chunk_pos;

	// Calculate terrain heights, and store to buffer
	int32_t global_cx = (int32_t)chunk_pos.x * (int32_t)chunk_size.x;
	int32_t global_cy = (int32_t)chunk_pos.y * (int32_t)chunk_size.z;

	for (uint16_t x = 0; x < chunk_size.x; ++x) {
		for (uint16_t y = 0; y < chunk_size.z; ++y) {
			int32_t cx = 100 + global_cx + (int32_t)x;
			int32_t cy = 100 + global_cy + (int32_t)y;

			chunk_buffer[x + (y * chunk_size.x)] = sea_level + (int16_t)(32.0f * sample_perlin_octaves(
				cx / 48.0f,                     // X
				cy / 48.0f,                     // Y
				5,                              // OCTAVES
				2.0f,                           // LACUNARITY
				0.47f));                        // PERSISTANCE
		}
	}

	//printf("\nBuffer finished");

	// fill inn with depths, 1: grass 4: dirt, rest stone for now then bedrock at bedrock
	// also: 0 in the noise is sea level, (make variable) but should be 63 so when ur standing on shore ur at 64 :D
	for (uint16_t x = 0; x < chunk_size.x; ++x) {
		for (uint16_t z = 0; z < chunk_size.z; ++z) {
			// STore the same height for all y levels on same block in 2d
			uint16_t h = chunk_buffer[x + (z * chunk_size.x)];

			for (uint16_t y = 0; y < chunk_size.y; ++y) {
				vec3i16_t block_pos = { x, y, z };
				int32_t i = chunk_index * chunk_data_size + get_block_index(block_pos);

				// Set block
				int cb = BLOCK_AIR;

				// Set blocks at different levels
				//if (y == 0) {
				//	cb = BLOCK_BEDROCK;
				//} else if (y < h - 5) {
				//	cb = BLOCK_STONE;
				//} else if (y < h - 1) {
				//	cb = BLOCK_DIRT;
				//} else if (y < h) {
				//	cb = BLOCK_GRASS;
				//}

				//if (h < sea_level + 3 && (cb == BLOCK_GRASS || cb == BLOCK_DIRT))
				//	cb = BLOCK_SAND; // Sand at shore

				if (y == 0)
					cb = BLOCK_BEDROCK;
				if (y == 1)
					cb = BLOCK_SAND;
				if (y == 2)
					cb = BLOCK_GRASS;
				if (y == 255)
					cb = BLOCK_DIRT;
				if (y == 254)
					cb = BLOCK_STONE;

				chunk_data[i] = block_t_new(cb);
			}
		}
	}

	// Update status
	chunk_status[chunk_index] = CHUNK_LOADED; /// Finished, and ready to be used. 
	//printf("\nLoaded chunk %d %d at index %d succesfully! (%d)", chunk_pos.x, chunk_pos.y, chunk_index, chunk_index * chunk_data_size);

	return 0;
}

int unload_chunk(vec2i16_t chunk_pos) {
	int32_t i = get_chunk_index(chunk_pos);

	if (i == -1)	// Can't unload because its already unloaded
		return -1;

	chunk_status[i] = CHUNK_UNLOADED;

	//printf("\nUnloaded chunk %d %d at index %d chunk index %d", chunk_pos.x, chunk_pos.y, i, i * chunk_data_size);
	return 0;
}


// Block functions
// chunk_pos: position of chunk to be found, free: if the chunk needs to be empty or not
int32_t get_chunk_index(vec2i16_t chunk_pos) {
	for (uint16_t i = 0; i < num_chunks; ++i) {
		if (vec2i16_t_equals(chunk_pos, chunk_locs[i])) {
			if (chunk_status[i] == CHUNK_UNLOADED) {
				//printf("\nINDEX FINDER: Skipped chunk %d %d at index %d because its unloaded", chunk_pos.x, chunk_pos.y, i);
				return -1;
			}

			//printf("\nINDEX FINDER: Returning chunk %d %d (loaded)", chunk_pos.x, chunk_pos.y);
			return i;
		}
	}

	return -1; // did not find a chunk
}

int32_t next_chunk_index() {
	for (uint16_t i = 0; i < num_chunks; ++i) {
		if (chunk_status[i] == CHUNK_UNLOADED)
			return i;
	}

	return 0; // idk
}

int32_t get_block_index(vec3i16_t c_block_pos) {
	int32_t block_index = c_block_pos.x + (chunk_size.x * c_block_pos.y) + (chunk_size.x * chunk_size.y * c_block_pos.z);
	return block_index;
}


block_t get_block(vec3i32_t block_pos) {
	int32_t chunk_index = get_chunk_index(get_chunk_pos(block_pos));
	
	if (chunk_index == -1)
		return block_t_new(BLOCK_UNDEFINED); // no block so undefined

	// Fine so we find
	chunk_index *= chunk_data_size;
	int32_t block_index = get_block_index(get_block_in_chunk_pos(block_pos));
	
	block_t block = chunk_data[chunk_index + block_index];
	return block;
}


// UTIL
vec2i16_t get_chunk_pos(vec3i32_t block_pos) {
	//printf("\nI got %d %d", block_pos.x, block_pos.z);
	return (vec2i16_t) { block_pos.x / (int32_t)chunk_size.x, block_pos.z / (int32_t)chunk_size.z };
}

vec3i16_t get_block_in_chunk_pos(vec3i32_t block_pos) {
	int16_t x = block_pos.x % chunk_size.x;
	int16_t y = block_pos.y;
	int16_t z = block_pos.z % chunk_size.z;

	vec3i16_t c_block_pos = { x, y, z };

	return c_block_pos;
}

uint16_t get_total_loaded_chunks() {
	uint16_t tot = 0;

	for (uint16_t i = 0; i < num_chunks; ++i) {
		if (chunk_status[i] == CHUNK_LOADED)
			++tot;
	}

	return tot;
}

// More chunk funcs
int load_bounds(vec2i16_t pos) {
	for (int16_t x = -render_distance; x <= render_distance; ++x) {
		for (int16_t y = -render_distance; y <= render_distance; ++y) {
			load_chunk((vec2i16_t) { x + pos.x, y + pos.y });
		}
	}

	return 0;
}

int unload_bounds(vec2i16_t pos) {
	for (uint16_t i = 0; i < num_chunks; ++i) {
		vec2i16_t chunk_pos = chunk_locs[i];
		vec2i16_t diff = { abs(pos.x - chunk_pos.x), abs(pos.y - chunk_pos.y) };

		if (chunk_status[i] == CHUNK_LOADED && (diff.x > render_distance || diff.y > render_distance)) {
			// If chunk is loaded, and is outside our renderdistance bounding box, unload it
			unload_chunk(chunk_pos);
		}
	}

	return 0;
}