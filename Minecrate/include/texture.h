#ifndef TEXTURE_H
#define TEXTURE_H

#include <raylib.h>
#include "block.h"

// Resolution of each block texture
static uint16_t texture_resolution = 16;

// Chosen texture pack
static char texture_pack[32] = "default";

// Directory to texture packs
static const char texture_packs_dir[32] = "assets/texture_packs";


Image texture_atlas_img;
Texture texture_atlas;

// Allocate memory
int init_texture_atlas();

// Generate the atlas and calculate size and stuff
int generate_texture_atlas();

// Get texcords in atlas
Vector2 get_texcoords_atlas(block_t block, side_t side);

// Get size of one block in texture atlas, in coordinate units
Vector2 get_texcoord_block_size();

// Deallocate
int free_texture_atlas();

#endif