#ifndef TEXTURE_H
#define TEXTURE_H

#include <raylib.h>
#include "block.h"

static const char block_textures_dir[32] = "res/block_textures";


Image texture_atlas_img;
Texture texture_atlas;

// Allocate memory
int init_texture_atlas();

// Generate the atlas and calculate size and stuff
int generate_texture_atlas();

// Get texcords in atlas
//Vector2 get_texcoords_atlas(block_t block);

// Get size of one block in texture atlas, in coordinate units
//Vector2 get_texcoord_block_size();

// Deallocate
int free_texture_atlas();

#endif