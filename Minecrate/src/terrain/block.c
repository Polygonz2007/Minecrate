#include "block.h"

block_t block_t_new(uint8_t type) {
	return (block_t) { type };
}

side_t side_t_new(uint8_t side) {
	return (side_t) { side };
}

side_t side_t_new_from_normal(vec3i8_t normal) {
	for (uint8_t i = 0; i < num_sides; ++i) {
		if (vec3i8_t_equals(normal, side_normals[i]))
			return side_t_new(i);
	}

	return side_t_new(SIDE_NONE);
}