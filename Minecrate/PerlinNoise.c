#include "PerlinNoise.h"

#ifndef RAYLIB_H
#include <raylib.h>
#include <math.h>
#endif

float Dot(Vector2 a, Vector2 b) {
	return a.x * b.x + a.y * b.y;
}

Vector2 Randdir(float x, float y) {
	SetRandomSeed(x * y + x);

	double r = (double)rand() / 5215.0f; // random number range 0 -> 2*PI
	double rx = cos(r); double ry = sin(r);

	return (Vector2) { rx, ry };
}

float SamplePerlin(float x, float y) {

	float remx = modf(x, 1), remy = modf(y, 1);
	
	// Corners
	Vector2 d00 = Randdir(x - remx, y - remy);
	Vector2 d01 = Randdir(x - remx, y + remy);
	Vector2 d10 = Randdir(x + remx, y - remy);
	Vector2 d11 = Randdir(x + remx, y + remy);

	return 1.0f;
}