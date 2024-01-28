
#include "chunk.h"
#include "block.h"
#include "vec3.h"

// Memory functions
int init_chunks() {
	num_chunks = renderdistance * 2 + 1;
}

int free_chunks() {

}


// Block functions
int32_t get_chunk_index(vec3i16_t chunk_pos) {
	for (uint16_t i = 0; i < num_chunks; ++i) {

	}
}

block_t get_block(uint32_t x, uint16_t y, uint32_t z) {
	vec3i16_t chunk_pos = { floor(x % 16), floor(z % 16) };
	int32_t chunk_pos_arr = get_chunk_index(chunk_pos);

	if (chunk_pos_arr != -1) {	// The chunk is valid and loaded

	}
}
