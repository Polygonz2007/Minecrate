
#include <stdio.h>
#include <raylib.h>
#include "texture.h"



// Calculate size and generate
int init_texture_atlas() {
	// Init as blank image
	texture_atlas_img = GenImageColor(
		texture_resolution * 6,
		num_block_types * texture_resolution,
		BLANK
	);

	texture_atlas = LoadTextureFromImage(texture_atlas_img);

	return 0;
}

// Fill in with blocks
int generate_texture_atlas() {
	// get texture pack and generate directory
	char tex_dir[64];
	snprintf(tex_dir, 63, "%s/%s", texture_packs_dir, texture_pack);

	// Fill in image with blocks
	for (uint8_t block = 0; block < num_block_types; ++block) {
		for (uint8_t side = 0; side < 6; ++side) {
			// Find texture for this side on this block and paste to atlas
			char dir[64];
			snprintf(dir, 63, "%s/%s_%s.png", tex_dir, block_names[block], side_names[side + 1]);

			// Get
			Image img;
			_Bool found = false;

			if (FileExists(dir)) {
				img = LoadImage(dir);
				found = true;
			} else {
				snprintf(dir, 63, "%s/%s.png", tex_dir, block_names[block]);
			}

			// If we cant load the one with side, try one for all sides
			if (!found && FileExists(dir)) {
				img = LoadImage(dir);
				found = true;
			}
			
			if (!found) {
				// If this doesnt exit either, make check textuere
				img = GenImageChecked(
					texture_resolution,
					texture_resolution,
					8, 8,
					BLACK,
					(Color) {
					255, 0, 255, 255
				});
			}

			// Draw to atlas
			const Rectangle textureRes = { 0, 0, texture_resolution, texture_resolution };
			const Rectangle destination = {
				side * texture_resolution, block * texture_resolution,
				texture_resolution, texture_resolution };

			ImageDraw(&texture_atlas_img, img, textureRes, destination, WHITE);
		}
	}

	texture_atlas = LoadTextureFromImage(texture_atlas_img);

	//GenTextureMipmaps(&texture_atlas);
	SetTextureFilter(texture_atlas, TEXTURE_FILTER_POINT);
	SetTextureWrap(texture_atlas, TEXTURE_WRAP_CLAMP);

	printf("\n\nLoaded texture atlas successfully.\n");

	return 0;
}

// Get texcords in atlas
Vector2 get_texcoords_atlas(block_t block, side_t side) {
	uint8_t id = block.type;

	Vector2 vec = { 
		(side.i - 1) * 0.166666f, // 6 sides total
		id / (double)num_block_types 
	};

	return vec;
}

// Get size of one block in texture atlas, in coordinate units
Vector2 get_texcoord_block_size() {
	// Return it
	return (Vector2) {
		0.1666f,
		1.0f / (double)num_block_types
	};
}

// Deallocate
int free_texture_atlas() {
	UnloadTexture(texture_atlas);
	UnloadImage(texture_atlas_img);

	return 0;
}
