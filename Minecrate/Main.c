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
#define INFO_TITLE_COL (Color){ 255, 255, 255, 215 }
#define INFO_COL (Color){ 255, 255, 255, 175 }

// Static
static float clamp(float d, float min, float max) {
    const float t = d < min ? min : d;
    return t > max ? max : t;
}

// Prototypes
void PlaceCube(int x, int y, int z);


int main()
{
    // WINDOW
    const int window_width = 2000;
    const int window_height = 1000;
    bool last_fullscreen = false;
    bool is_fullscreen = false;

    InitWindow(window_width, window_height, "Minecrate v0.1");

    DisableCursor();
    SetTargetFPS(60);
    SetConfigFlags(FLAG_MSAA_4X_HINT);

    // CAMERA
    Camera camera = { 0 };
    camera.position = (Vector3){ 2.0f, 1.83f, 2.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    float Sensitivity = 300.0f; // Higher sensitivity --> Less rotation per px moved

    // PHYSICS
    const float gravity = -9.81f;

    // PLAYER
    Vector3 position = { 0.5f, 0.0f, 0.5f }; // player position
    Vector2 look = { 0.707f, 0.0f }; // camera rotation in radians
    float vertical_velo = 0.0f;

    float PlayerHeight = 1.83f; // y offset of camera relative to player pos
    float Speed = 3.8f; // unit / second
    float SprintMult = 2.0f;


    // TERRAIN (Chunk size: 16 x 64 x 16)
    int *terrain = malloc(32 * 32 * sizeof(int));

    for (int x = 0; x < 32; x++) {
        for (int y = 0; y < 32; y++) {
            terrain[x + (y * 32)] = sample_perlin((float)x / 8.0f, (float)y / 8.0f) * 4.0f;
        }
    }

    position.y = (float)terrain[0] + 1.0f;

    const int start_time = time(NULL);


    // Main game loop
    while (!WindowShouldClose())
    {
        // WINDOW
        if (IsKeyPressed(KEY_F11)) { is_fullscreen = !is_fullscreen; }

        if (is_fullscreen != last_fullscreen) {
            if (is_fullscreen) {
                const int monitor = GetCurrentMonitor();
                SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
                ToggleBorderlessWindowed();
            } else {
                ToggleBorderlessWindowed();
                SetWindowSize(window_width, window_height);
            }

            last_fullscreen = is_fullscreen;
        }


        // Deltatime
        srand(start_time);
        float dt = GetFrameTime();

        // My beloved FPS string
        int fps = GetFPS();
        char fps_string[12];
        snprintf(fps_string, 11, "fps: %d", fps);


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

        if (IsKeyPressed(KEY_SPACE)) {
            vertical_velo += 5.0f;
        }

        // Gravity
        const int ix = (int)(position.x);
        const int iy = (int)(position.z);

        vertical_velo += gravity * dt;
        position.y += vertical_velo * dt;

        if (ix >= 0 && ix < 32 && iy >= 0 && iy < 32) {
            const int h = terrain[ix + (iy * 32)] + 1;

            if ((float)h >= position.y) {
                position.y = (float)h;
                vertical_velo = 0.0f;
            }
        }
        else {
            if (0.0f >= position.y) {
                position.y = 0.0f;
                vertical_velo = 0.0f;
            }
        }

        // Info
        char position_string[64];
        snprintf(position_string, 63, "position: %d, %d, %d", (int)floor(position.x), (int)floor(position.y), (int)floor(position.z));

        // CAMERA
        camera.position = (Vector3){ position.x, position.y + PlayerHeight, position.z };

        Vector2 md = GetMouseDelta();
        look.x += md.x / Sensitivity;
        look.y += md.y / Sensitivity;

        look.x = fmodf(look.x, PI * 2);
        look.y = clamp(look.y, -1.57f, 1.57f); // ca. 89 degrees in radians

        Vector3 CP = camera.position;
        camera.target = (Vector3){ CP.x + cos(look.x), CP.y - tan(look.y), CP.z + sin(look.x) };


        // DRAW
        BeginDrawing();
        ClearBackground(BLACK);//SKYBLUE);

        // 3D
        BeginMode3D(camera);

        // Chunk (put in function later)
        for (int x = 0; x < 32; x++) {
            for (int y = 0; y < 32; y++) {
                PlaceCube(x, terrain[x + (y * 32)], y);
            }
        }

        // WATER
        DrawPlane((Vector3) { 0.0f, -0.001f, 0.0f }, (Vector2) { 128.0f, 128.0f }, (Color) {
            0, 121, 241, 20
        });
        
        // Draw gizmos
        DrawGrid(16, 1.0f);
        DrawLine3D((Vector3) { 0.0f, 0.0f, 0.0f }, (Vector3) { 8.0f, 0.0f, 0.0f }, RED);
        DrawLine3D((Vector3) { 0.0f, 0.0f, 0.0f }, (Vector3) { 0.0f, 0.0f, 8.0f }, BLUE);

        EndMode3D();

        // UI
        DrawText("Minecrate v0.1", 10, 10, 30, WHITE);

        DrawText("-- performance --", 10, 50, 20, INFO_TITLE_COL);
        DrawText(fps_string, 10, 70, 20, INFO_COL);

        DrawText("-- player --", 10, 110, 20, INFO_TITLE_COL);
        DrawText(position_string, 10, 130, 20, INFO_COL);

        EndDrawing();
    }

    free(terrain);
    CloseWindow();

    return 0;
}

void PlaceCube(int x, int y, int z) {
    Color col = LIME;

    if (y < 3) {
        col = GOLD;
    }

    DrawCube((Vector3) { x + 0.5f, y + 0.5f, z + 0.5f }, 1.0f, 1.0f, 1.0f, (Color) {
        col.r + rand() / 128 / 16,
        col.g + rand() / 128 / 16,
        col.b + rand() / 128 / 16,
        255
    });
}
