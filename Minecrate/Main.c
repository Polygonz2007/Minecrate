// Minecrap.c : This file contains the 'main' function. Program execution begins and ends there.
//

#include <stdio.h>
#include <stdint.h>

#include <time.h>
#include <math.h>

#include <stdlib.h>
#include <raylib.h>
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
void PlaceCube(int x, int y, int z);


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
    Vector3 Position = { 16.0f, 32.0f, 16.0f }; // player position
    Vector2 Look = { 0.0f, 1.55f }; // camera rotation in radians

    float PlayerHeight = 1.83f; // y offset of camera relative to player pos
    float Speed = 3.8f; // unit / second
    float SprintMult = 2.0f;


    // TERRAIN (Chunk size: 16 x 64 x 16)
    int Terrain[32][32];

    for (int X = 0; X < 32; X++) {
        for (int Y = 0; Y < 32; Y++) {
            Terrain[X][Y] = 0;//noise2d((double)X, (double)Y) * 4;
        }
    }


    // Main game loop
    while (!WindowShouldClose())
    {
        // Deltatime
        srand(100); // keep random numbers across frames
        float DT = GetFrameTime();

        // My beloved FPS string
        int FPS = GetFPS();
        char FPSString[12];
        snprintf(FPSString, 11, "FPS: %d", FPS);


        // MOVEMENT
        float XM = cos(Look.x), YM = sin(Look.x);

        float CS = Speed;

        bool W, A, S, D;
        W = IsKeyDown(KEY_W); A = IsKeyDown(KEY_A); S = IsKeyDown(KEY_S); D = IsKeyDown(KEY_D);

        if ((W && A) || (W && D) || (S && A) || (S && D)) { CS *= 0.707f; }
        if (IsKeyDown(KEY_LEFT_SHIFT)) { CS *= SprintMult; }


        if (W) { Position.x += CS * DT * XM; Position.z += CS * DT * YM; }
        if (S) { Position.x += CS * DT * -XM; Position.z += CS * DT * -YM; }
        if (A) { Position.x += CS * DT * YM; Position.z += CS * DT * -XM; }
        if (D) { Position.x += CS * DT * -YM; Position.z += CS * DT * XM; }

        int ix = (int)Position.x;
        int iy = (int)Position.y;

        if (IsKeyPressed(KEY_SPACE)) {
            Position.y++;
        }

        if (IsKeyPressed(KEY_M)) {
            Position.y--;
        }

        char PositionString[64];
        snprintf(PositionString, 63, "Position: %d, %d, %d", (int)Position.x, (int)Position.y, (int)Position.z);

        // CAMERA
        camera.position = (Vector3){ Position.x, Position.y + PlayerHeight, Position.z };

        Vector2 MD = GetMouseDelta();
        Look.x += MD.x / Sensitivity;
        Look.y += MD.y / Sensitivity;

        Look.x = fmodf(Look.x, PI * 2);
        Look.y = clamp(Look.y, -1.55f, 1.55f); // ca. 89 degrees in radians

        Vector3 CP = camera.position;
        camera.target = (Vector3){ CP.x + cos(Look.x), CP.y - tan(Look.y), CP.z + sin(Look.x) };


        // DRAW
        BeginDrawing();
        ClearBackground(DARKBLUE);


        // 3D
        BeginMode3D(camera);

        // Chunk (put in function later)
        for (int X = 0; X < 32; X++) {
            for (int Y = 0; Y < 32; Y++) {
                PlaceCube(X, Terrain[X][Y], Y);
            }
        }

        DrawPlane((Vector3) { 0.0f, 0.8f, 0.0f }, (Vector2) { 1024.0f, 1024.0f }, (Color) {
            102, 191, 255, 179
        });
        //DrawGrid(8, 16.0f);

        EndMode3D();


        // UI
        DrawText("Minecrap BETA", 10, 10, 30, RAYWHITE);
        DrawText(FPSString, 10, 50, 20, InfoCol);
        DrawText(PositionString, 10, 70, 20, InfoCol);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}

void PlaceCube(int x, int y, int z) {
    Color col = GRASS_COL;

    if (y < 3) {
        col = SAND_COL;
    }

    float a = SamplePerlin(x, y);

    DrawCube((Vector3) { x + 0.5f, y + 0.5f, z + 0.5f }, 1.0f, 1.0f, 1.0f, (Color) { a * 255, a * 255, 0, 255 });
}