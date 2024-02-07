#include "perlin_noise.h"

#ifndef RAYLIB_H
#include <raylib.h>
#include <raymath.h>
#include <math.h>
#endif

#include <stdio.h>

static float lerpn(float a, float b, float t) {
	return a + (b - a) * t;
}

static float dot(Vector2 a, Vector2 b) {
	return a.x * b.x + a.y * b.y;
}

static float randdir(int x, int y) {
	x += y * 1000;

    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;

    double r = (float)x / 2147483648.0f; // to float, 0 - 1

	return r * r * (3.0f - 2.0f * r);
}

float sample_perlin(float x, float y) {

	const double remx = fmodf(x, 1.0f);
	const double remy = fmodf(y, 1.0f);

	// Corners
	const int xx = (int)floor(x), yy = (int)floor(y);
	const float A = randdir(xx, yy);
	const float B = randdir(xx, yy + 1);
	const float C = randdir(xx + 1, yy);
	const float D = randdir(xx + 1, yy + 1);

	// Interpolate
	const float AC = lerpn(A, C, remx); // Top
	const float BD = lerpn(B, D, remx); // Bottom
	float Result   = lerpn(AC, BD, remy);

	// Clamp
	if (Result < -1.0f)
		Result = -1.0f; 

	if (Result > 1.0f)
		Result = 1.0f;

	return Result;
}

float sample_perlin_octaves(float x, float y, int octaves, float lacunarity, float persistance) {
	float scale = 1.0f;
	float strength = 1.0f;

	float tot = 0.0f;
	float result = 0.0f;

	for (int i = 0; i < octaves; ++i) {
		result += sample_perlin(x * scale, y * scale) * strength;
		tot += strength;

		scale *= lacunarity;
		strength *= persistance;
	}

	return result / tot;
}