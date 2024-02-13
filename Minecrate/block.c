#include "block.h"

block_t block_t_new(uint8_t type) {
	return (block_t) { type };
}