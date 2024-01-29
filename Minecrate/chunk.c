
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
	vec2i16_t chunk_pos = { floor(block_pos.x / 16), floor(block_pos.z / 16) };
	int32_t chunk_index = get_chunk_index(chunk_pos);
}