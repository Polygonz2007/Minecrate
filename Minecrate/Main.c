// Minecrap.c : This file contains the 'main' function. Program execution begins and ends there.
//

#include <stdio.h>
#include <stdint.h>

#include <time.h>
#include <math.h>

#include <stdlib.h>
#include <raylib.h>
#include <raymath.h>

#include "PerlinNoise.h" // a mimir


// COLORS
#define GRASS_COL (Color){ 40, 200, 50, 255 };
#define SAND_COL (Color){ 240, 220, 150, 255 };


// Static
static float clamp(float d, float min, float max) {
    const float t = d < min ? min : d;
    return t > max ? max : t;
}

// Prototypes
void PlaceCube(int x, int y, int z, int t);


int main()
{
    // COLORS
    Color InfoCol = (Color){ 255, 255, 255, 175 };

    // WINDOW
    const int screenWidth = 2000;
    const int screenHeight = 1200;

    InitWindow(screenWidth, screenHeight, "Minecrap");

    DisableCursor();
    SetTargetFPS(60);


    // CAMERA
    Camera camera = { 0 };
    camera.position = (Vector3){ 2.0f, 1.83f, 2.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    float Sensitivity = 300.0f; // Higher sensitivity --> Less rotation per px moved


    // PLAYER
    Vector3 position = { 16.0f, 32.0f, 16.0f }; // player position
    Vector2 look = { 0.0f, 1.55f }; // camera rotation in radians

    float PlayerHeight = 1.83f; // y offset of camera relative to player pos
    float Speed = 3.8f; // unit / second
    float SprintMult = 2.0f;


    // TERRAIN (Chunk size: 16 x 64 x 16)
    int* terrain = malloc(64 * 64 * sizeof(int));

    for (int x = 0; x < 64; x++) {
        for (int y = 0; y < 64; y++) {
            terrain[x + (y * 64)] = (int)(SamplePerlin((float)x / 16.0f, (float)y / 16.0f) * 255.0f);
        }
    }


    // Main game loop
    while (!WindowShouldClose())
    {
        // Deltatime
        srand(100); // keep random numbers across frames
        float dt = GetFrameTime();

        // My beloved FPS string
        int FPS = GetFPS();
        char FPSString[12];
        snprintf(FPSString, 11, "FPS: %d", FPS);


        // MOVEMENT
        float xm = cos(look.x), ym = sin(look.x);

        float cs = Speed;

        bool w, a, s, d;
        w = IsKeyDown(KEY_W); a = IsKeyDown(KEY_A); s = IsKeyDown(KEY_S); d = IsKeyDown(KEY_D);

        if ((w && a) || (w && d) || (s && a) || (s && d)) { cs *= 0.707f; }
        if (IsKeyDown(KEY_LEFT_SHIFT)) { cs *= SprintMult; }


        if (w) { position.x += cs * dt * xm; position.z += cs * dt * ym; }
        if (s) { position.x += cs * dt * -xm; position.z += cs * dt * -ym; }
        if (a) { position.x += cs * dt * ym; position.z += cs * dt * -xm; }
        if (d) { position.x += cs * dt * -ym; position.z += cs * dt * xm; }

        int ix = (int)position.x;
        int iy = (int)position.y;

        if (IsKeyPressed(KEY_SPACE)) {
            position.y++;
        }

        if (IsKeyPressed(KEY_M)) {
            position.y--;
        }

        char position_string[64];
        snprintf(position_string, 63, "position: %d, %d, %d", (int)position.x, (int)position.y, (int)position.z);

        // CAMERA
        camera.position = (Vector3){ position.x, position.y + PlayerHeight, position.z };

        Vector2 md = GetMouseDelta();
        look.x += md.x / Sensitivity;
        look.y += md.y / Sensitivity;

        look.x = fmodf(look.x, PI * 2);
        look.y = clamp(look.y, -1.55f, 1.55f); // ca. 89 degrees in radians

        Vector3 CP = camera.position;
        camera.target = (Vector3){ CP.x + cos(look.x), CP.y - tan(look.y), CP.z + sin(look.x) };


        // DRAW
        BeginDrawing();
        ClearBackground(DARKBLUE);


        // 3D
        BeginMode3D(camera);

        // Chunk (put in function later)
        for (int x = 0; x < 64; x++) {
            for (int y = 0; y < 64; y++) {
                PlaceCube(x, terrain[x + (y * 64)] / 32, y, terrain[x + (64 * y)]); //terrain[x + (y * 64)], y); 
            }
        }

        //DrawPlane((Vector3) { 0.0f, 0.8f, 0.0f }, (Vector2) { 1024.0f, 1024.0f }, (Color) {
        //    102, 191, 255, 179
        //});

        DrawGrid(8, 16.0f);

        EndMode3D();


        // UI
        DrawText("Minecrap BETA", 10, 10, 30, RAYWHITE);
        DrawText(FPSString, 10, 50, 20, InfoCol);
        DrawText(position_string, 10, 70, 20, InfoCol);

        EndDrawing();
    }

    free(terrain);
    CloseWindow();

    return 0;
}

void PlaceCube(int x, int y, int z, int t) {
    Color col = GRASS_COL;

    if (y < 3) {
        col = SAND_COL;
    }

    DrawCube((Vector3) { x + 0.5f, y + 0.5f, z + 0.5f }, 1.0f, 1.0f, 1.0f, (Color) {
        t,
        0,
        0,
        255
    });
}