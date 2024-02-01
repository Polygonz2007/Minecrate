#include "block.h"

block_t block_t_new(uint8_t type) {
	return (block_t) { type };
}

block_t block_t_new_from_name(char type[]) {
	for (uint16_t i = 0; i < num_block_types; ++i) {
		if (block_names[i] == type)
			return block_t_new(i);
	}

	return block_t_new(0);
}