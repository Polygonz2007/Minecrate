#include "PerlinNoise.h"

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
	srand(x);
	srand(rand() - y);

	//const double r = (double)rand() / 5215.0f; // random number range 0 -> 2*PI
	//const double rx = cos(r);
	//const double ry = sin(r);

	const double m = 2.0f;
	return -1.0f + modf((float)rand() / 256.0f, &m);
}

float sample_perlin(float x, float y) {

	const double ix = 1.0f, iy = 1.0f;
	const double remx = modf(x, &ix), remy = modf(y, &iy);

	// Corners
	const int xx = (int)x, yy = (int)y;
	const float A = randdir(xx, yy);
	const float B = randdir(xx, yy + 1);
	const float C = randdir(xx + 1, yy);
	const float D = randdir(xx + 1, yy + 1);

	// Interpolate
	const float AC = lerpn(A, C, remx); // Top
	const float BD = lerpn(B, D, remx); // Bottom
	float Result   = lerpn(AC, BD, remy);

	// Clamp
	//if (Result < -1.0f) { Result = -1.0f; }
	//if (Result > 1.0f)  { Result = 1.0f; }

	return Result;
}