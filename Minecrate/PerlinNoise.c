#include "PerlinNoise.h"

#ifndef RAYLIB_H
#include <raylib.h>
#include <raymath.h>
#include <math.h>
#endif

#include <stdio.h>

static float Lerpn(float a, float b, float t) {
	return a + (b - a) * t;
}

static float Dot(Vector2 a, Vector2 b) {
	return a.x * b.x + a.y * b.y;
}

static float Randdir(int x, int y) {
	srand(x + y);
	srand(rand() - y * 10);
	srand(rand());

	//const double r = (double)rand() / 5215.0f; // random number range 0 -> 2*PI
	//const double rx = cos(r);
	//const double ry = sin(r);

	return 1.0f + (float)rand() / 16384.0f;
}

float SamplePerlin(float x, float y) {

	const double ix, iy;
	const double remx = modf(x, &ix), remy = modf(y, &iy);


	// Corners
	int xx = (int)x, yy = (int)y;
	const float A = Randdir(xx, yy);
	const float B = Randdir(xx, yy + 1);
	const float C = Randdir(xx + 1, yy);
	const float D = Randdir(xx + 1, yy + 1);

	// Vectors to pixel
	//const Vector2 TL = {remx, remy};
	//const Vector2 BL = { remx, 1 - remy };
	//const Vector2 TR = { 1 - remx, remy };
	//const Vector2 BR = { 1 - remx, 1 - remy };

	// Dots
	//const float A = Dot(d00, TL);
	//const float B = Dot(d01, BL);
	//const float C = Dot(d10, TR);
	//const float D = Dot(d11, BR);

	// Interpolate
	const float AC = Lerpn(A, C, remx); // Top
	const float BD = Lerpn(B, D, remx); // Bottom
	float Result   = Lerpn(AC, BD, remy);

	// Clamp
	//if (Result < -1.0f) { Result = -1.0f; }
	//if (Result > 1.0f)  { Result = 1.0f; }

	return Result;
}