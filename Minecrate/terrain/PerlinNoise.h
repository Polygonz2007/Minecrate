#ifndef PERLIN_H
#define PERLIN_H

float sample_perlin(float x, float y);

float sample_perlin_octaves(float x, float y, int octaves, float lacunarity, float persistance);

#endif