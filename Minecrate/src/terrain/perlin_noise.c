#include "perlin_noise.h"

#include <raylib.h>
#include <raymath.h>
#include <math.h>

#include <stdio.h>

static inline float lerpn(float a, float b, float t) {
	return a + (b - a) * t;
}

static inline float dot(Vector2 a, Vector2 b) {
	return a.x * b.x + a.y * b.y;
}

static float randdir(int x, int y, int z) {
	x += y * 100;
	x -= z * 827;
	
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
	const float A = randdir(xx, yy, 0);
	const float B = randdir(xx, yy + 1, 0);
	const float C = randdir(xx + 1, yy, 0);
	const float D = randdir(xx + 1, yy + 1, 0);

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





float sample_perlin_3d(float x, float y, float z) {

	const double remx = fmodf(x, 1.0f);
	const double remy = fmodf(y, 1.0f);
	const double remz = fmodf(z, 1.0f);

	// Corners
	const int xx = (int)floor(x), yy = (int)floor(y), zz = (int)floor(z);
	const float A = randdir(xx, yy, zz);
	const float B = randdir(xx, yy + 1, zz);
	const float C = randdir(xx + 1, yy, zz);
	const float D = randdir(xx + 1, yy + 1, zz);
	const float E = randdir(xx, yy, zz + 1);
	const float F = randdir(xx, yy + 1, zz +1);
	const float G = randdir(xx + 1, yy, zz + 1);
	const float H = randdir(xx + 1, yy + 1, zz + 1);

	// Interpolate
	const float AC = lerpn(A, C, remx); // Top
	const float BD = lerpn(B, D, remx); // Bottom
	const float EF = lerpn(E, F, remx); // Top
	const float GH = lerpn(G, H, remx); // Bottom
	float ResultA = lerpn(AC, BD, remy);
	float ResultB = lerpn(EF, GH, remy);
	float Result = lerpn(ResultA, ResultB, remy);

	// Clamp
	if (Result < -1.0f)
		Result = -1.0f;

	if (Result > 1.0f)
		Result = 1.0f;

	return Result;
}

float sample_perlin_octaves_3d(float x, float y, float z, int octaves, float lacunarity, float persistance) {
	float scale = 1.0f;
	float strength = 1.0f;

	float tot = 0.0f;
	float result = 0.0f;

	for (int i = 0; i < octaves; ++i) {
		result += sample_perlin_3d(x * scale, y * scale, z * scale) * strength;
		tot += strength;

		scale *= lacunarity;
		strength *= persistance;
	}

	return result / tot;
}