#include "chunk.h"

block_t GetBlock(int x, int y, int z) {
    int cx = floor(x / 16), cy = floor(z / 16);
    x = x % 16;
    z = z % 16;

    return (block_t) { 0 };
}
