// Minecrap.c : This file contains the 'main' function. Program execution begins and ends there.
//

#include <stdio.h>
#include <stdint.h>

#include <time.h>
#include <math.h>

#include <stdlib.h>
#include <raylib.h>
#include <raymath.h>

#include "perlinNoise.h" // a mimir
#include "meshes.h" // dos mimires


// COLORS
#define INFO_TITLE_COL (Color){ 255, 255, 255, 215 }
#define INFO_COL (Color){ 255, 255, 255, 175 }

#define GRASS (Color){ 50, 180, 60, 255 }
#define SAND (Color){ 240, 230, 170, 255 }

// Static
static float clamp(float d, float min, float max) {
    const float t = d < min ? min : d;
    return t > max ? max : t;
}

static int clampint(int d, int min, int max) {
    const int t = d < min ? min : d;
    return t > max ? max : t;
}

// Prototypes
void PlaceCube(int x, int y, int z);


int main()
{
    // WINDOW
    const int default_window_width = 2000;
    const int default_window_height = 1000;
    int window_width = default_window_width;
    int window_height = default_window_height;
    bool last_fullscreen = false;
    bool is_fullscreen = false;

    InitWindow(default_window_width, default_window_height, "Minecrate v0.1");

    DisableCursor();
    SetTargetFPS(60);
    SetConfigFlags(FLAG_MSAA_4X_HINT);

    // CAMERA
    Camera camera = { 0 };
    camera.position = (Vector3){ 2.0f, 1.83f, 2.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 80.0f;
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


    // GAMEPLAY
    int hotbar_item_size = 80; // px
    int hotbar_selected = 0; // 0 - 8


    // TERRAIN (Chunk size: 16 x 64 x 16)int 
    int chunk_size = 196; // size of x and z in chunk
    int *terrain = malloc(chunk_size * chunk_size * sizeof(int));

    for (int x = 0; x < chunk_size; x++) {
        for (int y = 0; y < chunk_size; y++) {
            terrain[x + (y * chunk_size)] = -2 + 24.0f * sample_perlin_octaves(
                        100.0f + (float)x / 32.0f,      // X
                        (float)y / 32.0f,               // Y
                        3,                              // OCTAVES
                        1.8f,                           // LACUNARITY
                        0.4f)                          // PERSISTANCE
                - sample_perlin((float)x / 154.0f, (float)y / 154.0f) * 60.0f;
        }
    }

    position.y = (float)terrain[0] + 1.0f;

    const int start_time = time(NULL);

    Mesh mush = GenPlate();
    Model model = LoadModelFromMesh(mush);

    Image img = GenImageWhiteNoise(64, 64, 0.5f);
    Texture texture = LoadTextureFromImage(img);
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;

    // Main game loop
    while (!WindowShouldClose())
    {
        // WINDOW
        if (IsKeyPressed(KEY_F11)) { is_fullscreen = !is_fullscreen; }

        if (is_fullscreen != last_fullscreen) {
            if (is_fullscreen) {
                const int monitor = GetCurrentMonitor();
                window_width = GetMonitorWidth(monitor);
                window_height = GetMonitorHeight(monitor);

                SetWindowSize(window_width, window_height);
                ToggleBorderlessWindowed();
            } else {
                ToggleBorderlessWindowed();

                window_width = default_window_width;
                window_height = default_window_height;

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
            vertical_velo += 4.0f;
        }

        const float scroll = floor(GetMouseWheelMove());
        
        if (scroll < 0.0f) {
            --hotbar_selected;
            if (hotbar_selected < 0) { hotbar_selected = 8; }
        } else if (scroll > 0.0f) {
            ++hotbar_selected;
            if (hotbar_selected > 8) { hotbar_selected = 0; }
        }
        

        // Gravity
        const int ix = (int)(position.x);
        const int iy = (int)(position.z);

        vertical_velo += gravity * dt;
        position.y += vertical_velo * dt * 2;

        if (ix >= 0 && ix < chunk_size && iy >= 0 && iy < chunk_size) {
            const int h = terrain[ix + (iy * chunk_size)] + 1;

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
        look.y = clamp(look.y, -1.57f, 1.57f);

        Vector3 CP = camera.position;
        camera.target = (Vector3){ CP.x + cos(look.x), CP.y - tan(look.y), CP.z + sin(look.x) };


        // DRAW
        BeginDrawing();
        ClearBackground(SKYBLUE);

        // 3D
        BeginMode3D(camera);

        // Chunk (put in function later)
        for (int x = 0; x < chunk_size; x++) {
            for (int y = 0; y < chunk_size; y++) {
                PlaceCube(x, terrain[x + (y * chunk_size)], y);
            }
        }

        // WATER
        DrawPlane((Vector3) { 0.0f, -0.2f, 0.0f }, (Vector2) { 512.0f, 512.0f }, (Color) {
            0, 121, 241, 200
        });

        // Draw gizmos (TESTING ONLY)
        DrawGrid(16, 1.0f);
        DrawLine3D((Vector3) { 0.0f, 0.0f, 0.0f }, (Vector3) { 8.0f, 0.0f, 0.0f }, RED);
        DrawLine3D((Vector3) { 0.0f, 0.0f, 0.0f }, (Vector3) { 0.0f, 0.0f, 8.0f }, BLUE);

        //DrawModel(model, (Vector3) { 0.0f, 0.0f, 0.0f }, 1.0f, WHITE);

        EndMode3D();

        // --  UI  --
        // HOTBAR
        int center_horizontal = window_width / 2;
        int border = 4; // px

        for (int i = 0; i < 9; ++i) {
            int x = center_horizontal + (-4.5f + (float)i) * hotbar_item_size;
            DrawRectangle(x, window_height - hotbar_item_size, hotbar_item_size, hotbar_item_size, WHITE); // OUTLINE
            DrawRectangle(x + border, window_height - hotbar_item_size + border, hotbar_item_size - 2 * border, hotbar_item_size - 2 * border, hotbar_selected == i ? BLACK : GRAY); // OUTLINE
        }

        // INFO
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
    Color col = GRASS;

    if (y < sample_perlin((float)x / 10.0f + 1000.0f, (float)y / 10.0f) * 2.0f) {
        col = SAND;
    }

    DrawCube((Vector3) { x + 0.5f, y + 0.5f, z + 0.5f }, 1.0f, 1.0f, 1.0f, (Color) {
        clampint(col.r + rand() / 128 / 32, 0, 255),
        clampint(col.g + rand() / 128 / 32, 0, 255),
        clampint(col.b + rand() / 128 / 32, 0, 255),
        255
    });
}
