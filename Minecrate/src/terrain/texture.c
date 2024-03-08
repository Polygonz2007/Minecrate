
#include <stdio.h>
#include <raylib.h>
#include "texture.h"



// Calculate size and generate
int init_texture_atlas() {
	// Init as blank image
	texture_atlas_img = GenImageChecked(
		texture_resolution * 6,
		num_block_types * texture_resolution,
		8, 8,
		BLACK,
		(Color){255, 0, 255, 255}
	);

	texture_atlas = LoadTextureFromImage(texture_atlas_img);

	return 0;
}

// Fill in with blocks
int generate_texture_atlas() {
	// Fill in image with blocks
	for (uint8_t block = 0; block < num_block_types; ++block) {
		for (uint8_t side = 0; side < 6; ++side) {
			// Find texture for this side on this block and paste to atlas
			const char dir[48];
			snprintf(dir, 47, "%s/BLOCK_%s.png", block_textures_dir, block_names[block]); // change to /BLOCK_%s_%s when add sides

			// Get
			const Image img = LoadImage(dir);
			const Rectangle destination = {
				side * texture_resolution, block * texture_resolution,
				texture_resolution, texture_resolution };

			const Rectangle textureRes = { 0, 0, texture_resolution, texture_resolution };

			ImageDraw(&texture_atlas_img, img, textureRes, destination, WHITE);
		}
	}

	texture_atlas = LoadTextureFromImage(texture_atlas_img);

	printf("\n\nLoaded texture atlas successfully.\n");

	return 0;
}

// Get texcords in atlas
//Vector2 get_texcoords_atlas(block_t block);

// Get size of one block in texture atlas, in coordinate units
//Vector2 get_texcoord_block_size();

// Deallocate
int free_texture_atlas() {
	UnloadTexture(texture_atlas);
	UnloadImage(texture_atlas_img);

	return 0;
}
