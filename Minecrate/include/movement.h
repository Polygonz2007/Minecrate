
#ifndef MOVEMENT_H
#define MOVEMENT_H

#include <raylib.h>

// Data
Vector3 player_position = { 0.5f, 0.0f, 0.5f }; // player_position
Vector3 player_velocity = { 0, 0, 0 };
vec3i32_t int_pos = { 0, 0, 0 }; // Floored pos


// Settings
const float player_height = 1.77f; // y offset of camera relative to player pos
const float player_speed = 3.8f; // unit / second
const float player_jump_power = 8.0f;
const float player_sprint_multiplier = 2.0f;
const float player_radius = 0.5f; // must be less than 1

// Physics
const Vector3 gravity = { 0.0f, -9.81f, 0.0f }
_Bool on_ground = false;





// Prototypes
int update_input();
int update_velocity();
int update_position();


#endif