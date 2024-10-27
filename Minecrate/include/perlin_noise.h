#ifndef PERLIN_H
#define PERLIN_H

float sample_perlin(float x, float y);
float sample_perlin_3d(float x, float y, float z);

// Returns number between -1 and 1
float sample_perlin_octaves(float x, float y, int octaves, float lacunarity, float persistance);
float sample_perlin_octaves_3d(float x, float y, float z, int octaves, float lacunarity, float persistance);

#endif