
#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <stdbool.h>

#include "perlin_noise.h"

#include "vec2.h"
#include "vec3.h"

#include "chunk.h"
#include "block.h"
#include "mesh.h"

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
	chunk_status = calloc(num_chunks, sizeof(uint8_t));	// calloc bc all chunks unloaded
	chunk_buffer = malloc(chunk_size.x * chunk_size.z * sizeof(uint16_t));

	chunk_meshes = malloc(num_chunks * sizeof(Mesh));
	chunk_models = malloc(num_chunks * sizeof(Model));

	// Track memory
	chunk_memory_usage = 0;
	chunk_memory_usage += chunk_data_size * num_chunks * sizeof(block_t);
	chunk_memory_usage += num_chunks * sizeof(vec2i16_t);
	chunk_memory_usage += num_chunks * sizeof(uint8_t);
	chunk_memory_usage += chunk_size.x * chunk_size.z * sizeof(uint16_t);

	chunk_memory_usage += num_chunks * sizeof(Mesh);
	chunk_memory_usage += num_chunks * sizeof(Model);

	printf("Memory allocated.\nChunk data size: %d\nTotal memory usage: %lu\n", chunk_data_size, chunk_memory_usage);
	return 0;
}

int free_chunks() { // WARNING: any chunk functions including load_mesh or load_chunk should NOT be caled after this is called.
	// Free all allocated memory
	printf("\n\nFreeing chunk memory...\n");
	free(chunk_data);
	free(chunk_locs);
	free(chunk_status);
	free(chunk_buffer);

	free(chunk_meshes);
	free(chunk_models);

	printf("Finished freeing chunks.\n");
	return 0;
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
			int32_t cx = global_cx + (int32_t)x;
			int32_t cy = global_cy + (int32_t)y;
			// + 1000 to avoid problems temporariy

			// TERRAIN
			double erosion = 0.5f + sample_perlin_octaves(cx / 256.0f, cy / 256.0f, 3, 1.5f, 0.75f) * 0.5f;

			double main = sample_perlin_octaves(cx / 128.0f, cy / 128.0f, 9, 2.0f, 0.47f);
			double detail = sample_perlin_octaves(cx / 16.0f, cy / 16.0f, 4, 2.0f, 0.5f);
			double ocean = (-40.0f + sample_perlin_octaves(cx / 756.0f, cy / 756.0f, 2, 1.7f, 0.6f) * 90.0f);

			main = 1.0f - pow(main, 0.7f);
			main = (96.0f * main - 40.0f);

			detail *= 8.0f * erosion;

			chunk_buffer[x + (y * chunk_size.x)] = (uint16_t)(sea_level + main - ocean + detail);
		}
	}

	//printf("\nBuffer finished");

	// fill inn with depths, 1: grass 4: dirt, rest stone for now then bedrock at bedrock
	// also: 0 in the noise is sea level, (make variable) but should be 63 so when ur standing on shore ur at 64 :D
	for (uint16_t x = 0; x < chunk_size.x; ++x) {
		for (uint16_t z = 0; z < chunk_size.z; ++z) {
			// STore the same height for all y levels on same block in 2d
			int32_t wx = (chunk_pos.x * chunk_size.x) + x;
			int32_t wz = (chunk_pos.y * chunk_size.z) + z;

			uint16_t h = chunk_buffer[x + (z * chunk_size.x)];

			for (uint16_t y = 0; y < chunk_size.y; ++y) {
				vec3i32_t block_pos = {wx, y, wz };
				uint32_t i = chunk_index * chunk_data_size + get_block_index((vec3u16_t){x, y, z});

				// Set block
				int cb = BLOCK_AIR;

				// LAND
				if (y == 0) {
					cb = BLOCK_BEDROCK;
				} else if (y < h - 5) {
					cb = BLOCK_STONE;
				} else if (y < h - 1) {
					cb = BLOCK_DIRT;
				} else if (y < h) {
					cb = BLOCK_GRASS;
				}

				// Rocky mountains
				if (h > 95 && (cb == BLOCK_GRASS || cb == BLOCK_DIRT))
					cb = rand() % 2 == 0 ? BLOCK_COBBLESTONE : BLOCK_STONE;

				// OCEAN
				if (h < sea_level + 2 && (cb == BLOCK_GRASS || cb == BLOCK_DIRT))
					cb = BLOCK_SAND; // Sand at shore

				if (y < sea_level && cb == BLOCK_AIR)
					cb = BLOCK_WATER;

				
				// CAVES
				// 3d noise, abs(noise - 1.0f) > 0.05f
				//if (fabs(sample_perlin_octaves_3d(block_pos.x / 40.0f, block_pos.y / 30.0f, block_pos.z / 40.0f, 4, 1.8f, 0.5f)) < 0.2f)
					//cb = BLOCK_AIR;

				chunk_data[i] = block_t_new(cb);
			}
		}
	}

	// Trees + decorations
	//vec2u8_t tree_pos = { rand() % chunk_size.x, rand() % chunk_size.z };
	//uint16_t tree_height = chunk_buffer[tree_pos.x + (tree_pos.y * chunk_size.x)];

	//for (uint16_t x = tree_pos.x - 2; x < tree_pos.x + 2; ++x) {
	//	for (uint16_t z = tree_pos.y - 2; z < tree_pos.y + 2; ++z) {
	//		uint32_t i = chunk_index * chunk_data_size + get_block_index((vec3u16_t) { x, tree_height, z });
	//		uint8_t cb = BLOCK_OAK_LEAVES;
	//		if (x == tree_pos.x && z == tree_pos.y)
	//			cb = BLOCK_OAK_LOG;
	//
	//		chunk_data[i] = block_t_new(cb);
	//	}
	//}

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

int32_t get_block_index(vec3u16_t c_block_pos) {
	int32_t block_index = c_block_pos.x + (chunk_size.x * c_block_pos.y) + (chunk_size.x * chunk_size.y * c_block_pos.z);
	return block_index;
}


block_t get_block(vec3i32_t block_pos) {
	if (block_pos.y < 0 || block_pos.y > chunk_size.y)
		return block_t_new(BLOCK_UNDEFINED);

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
	return (vec2i16_t) { 
		floor((float)block_pos.x / chunk_size.x),
		floor((float)block_pos.z / chunk_size.z) 
	};
}

vec3u16_t get_block_in_chunk_pos(vec3i32_t block_pos) {
	// Modulo x and z
	vec2i16_t chunk_pos = get_chunk_pos(block_pos);
	int16_t ix = block_pos.x, iz = block_pos.z;

	ix -= chunk_pos.x * chunk_size.x;
	iz -= chunk_pos.y * chunk_size.z;

	// Unsign and return
	uint16_t x = (uint16_t)ix;
	uint16_t y = block_pos.y;
	uint16_t z = (uint16_t)iz;

	vec3u16_t c_block_pos = { x, y, z };

	return c_block_pos;
}

uint16_t get_total_loaded_chunks() {
	uint16_t tot = 0;

	for (uint16_t i = 0; i < num_chunks; ++i) {
		if (chunk_status[i] >= CHUNK_LOADED)
			++tot;
	}

	return tot;
}

// More chunk funcs
int load_bounds(vec2i16_t pos) {
	for (int16_t x = -render_distance; x <= render_distance; ++x) {
		for (int16_t y = -render_distance; y <= render_distance; ++y) {
			load_chunk((vec2i16_t) { x + pos.x, y + pos.y });

			if ((x > -render_distance && x < render_distance) && (y > -render_distance && y < render_distance)) {
				// Only load chunk mesh when surrounded by chunks on all sides
				load_chunk_mesh((vec2i16_t) { x + pos.x, y + pos.y });
			}
		}
	}

	return 0;
}

int unload_bounds(vec2i16_t pos) {
	for (uint16_t i = 0; i < num_chunks; ++i) {
		// Make sure the chunk is unloadable
		if (chunk_status[i] == CHUNK_UNLOADED || chunk_status[i] == CHUNK_LOADING)
			continue;

		// Unload if too far away
		vec2i16_t chunk_pos = chunk_locs[i];
		vec2i16_t diff = { abs(pos.x - chunk_pos.x), abs(pos.y - chunk_pos.y) };

		if (diff.x > render_distance || diff.y > render_distance) {
			unload_chunk_model_and_mesh(chunk_pos);
			unload_chunk(chunk_pos);
		}
	}

	return 0;
}