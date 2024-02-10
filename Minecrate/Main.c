
// FOR MULITHREADING
#define WIN32_LEAN_AND_MEAN
#if defined(_WIN32)
#define WIN32
#endif
#if defined(_WIN64)
#define WIN64
#define _AMD64_
#undef _X86_
#else
#undef _AMD64_
#define _X86_
#endif

#if defined(_WIN32)           
#define NOGDI             // All GDI defines and routines
#define NOUSER            // All USER defines and routines
#endif

#include <Windows.h> // or any library that uses Windows.h

#if defined(_WIN32)           // raylib uses these names as function parameters
#undef near
#undef far
#endif
// END MULTITHREADING

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <time.h>
#include <math.h>

#include <raylib.h>
#include <raymath.h>

#include "vec3.h"
#include "vec2.h"

#include "perlin_noise.h" // a mimir
#include "mesh.h" // dos mimires
#include "block.h" // tres mimires
#include "chunk.h" // finalmente mimir


// COLORS
#define INFO_TITLE_COL (Color){ 255, 255, 255, 215 }
#define INFO_COL (Color){ 255, 255, 255, 175 }
#define NIGHT (Color) { 0, 20, 80, 255 }

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
void place_cube(int x, int y, int z, block_t block);
DWORD WINAPI update_chunks(LPVOID lpParameter);
_Bool is_thread_running(HANDLE hThread);










// DEBUG
struct debug_settings {
    _Bool terrain_loading;
    _Bool display_info;
    _Bool test_environment;
    _Bool fly_mode;
    _Bool show_chunk_borders;
};

struct debug_settings debug = {
    .terrain_loading = true,
    .display_info = true,
    .test_environment = true,
    .fly_mode = true,
    .show_chunk_borders = true
};


// Main :D
int main() {

    // WINDOW
    const int default_window_width = 1600;
    const int default_window_height = 800;
    int window_width = default_window_width;
    int window_height = default_window_height;
    bool last_fullscreen = false;
    bool is_fullscreen = false;

    //Image Icon = LoadImage("./"); // add minecraftlogo av ael.com

    InitWindow(default_window_width, default_window_height, "Minecrate");
    //SetWindowIcon(Icon);

    DisableCursor();
    SetTargetFPS(30);
    SetConfigFlags(FLAG_MSAA_4X_HINT);

    // CAMERA
    Camera camera = { 0 };
    camera.position = (Vector3){ 2.0f, 1.83f, 2.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 80.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    float sensitivity = 300.0f; // Higher sensitivity --> Less rotation per px moved

    // PHYSICS
    const float gravity = -9.81f;


    // PLAYER SETTINGS AND DATA
    Vector3 position = { 0.5f, 0.0f, 0.5f }; // player position
    Vector2 look = { 0.707f, 0.0f }; // camera rotation in radians
    float vertical_velo = 0.0f;

    float player_height = 1.83f; // y offset of camera relative to player pos
    float player_speed = 3.8f; // unit / second
    float sprint_multiplier = 2.0f;


    // GAMEPLAY
    int hotbar_item_size = 80; // px
    int hotbar_selected = 0; // 0 - 8


    // TERRAIN
    vec2i16_t* lpArgPtr = malloc(sizeof(vec2i16_t));;
    *lpArgPtr = (vec2i16_t){ 0, 0 };

    HANDLE chunk_h_thread = NULL; // Multithreading for chunks
    DWORD chunk_dw_thread_id;
    _Bool chunk_thread_status = false; // fasle: open, true: running

    // Init and load terrain
    if (debug.terrain_loading) { 
        init_chunks();

        // Load starting chunks, on different thread
        chunk_h_thread = CreateThread(NULL, 0, update_chunks, lpArgPtr, 0, &chunk_dw_thread_id);
    }



    // MESH GEN TESTING
    Mesh mush = GenPlate();
    Model model = LoadModelFromMesh(mush);

    Image img = GenImagePerlinNoise(16, 16, 0, 0, 4.0f);
    ImageColorTint(&img, block_colors[BLOCK_GRASS]);

    Texture texture = LoadTextureFromImage(img);
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;


    // Performance
    _Bool start_loading_finished = false;
    const long start_time = time(NULL);


    // Main game loop
    while (!WindowShouldClose())
    {

        // WINDOW AND SETTINGS
        if (IsKeyPressed(KEY_F3))
            debug.display_info = !debug.display_info;

        if (IsKeyDown(KEY_F3) && IsKeyPressed(KEY_G))
            debug.show_chunk_borders = !debug.show_chunk_borders;

        if (IsKeyDown(KEY_F3) && IsKeyPressed(KEY_F))
            debug.fly_mode = !debug.fly_mode;

        if (IsKeyPressed(KEY_F11))
            is_fullscreen = !is_fullscreen;

        if (is_fullscreen != last_fullscreen) {
            if (is_fullscreen) {
                const int monitor = GetCurrentMonitor();
                window_width = GetMonitorWidth(monitor);
                window_height = GetMonitorHeight(monitor);

                SetWindowSize(window_width, window_height);
                ToggleBorderlessWindowed();
            }
            else {
                ToggleBorderlessWindowed();

                window_width = default_window_width;
                window_height = default_window_height;

                SetWindowSize(window_width, window_height);
            }

            last_fullscreen = is_fullscreen;
        }

        // Terrain loaodng blah blah
        _Bool chunk_thread_running = false;

        if (debug.terrain_loading)
            chunk_thread_running = is_thread_running(chunk_h_thread);

        if (!start_loading_finished && chunk_thread_running && debug.terrain_loading) {
            // Render loading screen
            BeginDrawing();
            ClearBackground(BLACK);

            char s[64];
            uint16_t tot_loaded_chunks = get_total_loaded_chunks();
            float loaded = ((float)tot_loaded_chunks / (float)num_chunks);
            snprintf(s, 63, "Loading world... (%.01f%%)", loaded * 100.0f);

            DrawRectangle(0, 0, loaded * window_width, window_height, RED);
            DrawText(s, 200, (window_height - 24) * 0.5f, 24, RAYWHITE);

            EndDrawing();

            continue;
        } else {
            start_loading_finished = true;
        }

        // Deltatime
        srand(start_time);
        const float dt = GetFrameTime();

        // My beloved FPS string
        const int fps = GetFPS();
        char fps_string[12];
        snprintf(fps_string, 11, "fps: %d", fps);
        

        // MOVEMENT
        const float xm = cos(look.x), ym = sin(look.x);
        float cs = player_speed; // Current player_speed, for multipliers and stuff

        bool w, a, s, d;
        w = IsKeyDown(KEY_W);
        a = IsKeyDown(KEY_A);
        s = IsKeyDown(KEY_S);
        d = IsKeyDown(KEY_D);

        if ((w && a) || (w && d) || (s && a) || (s && d)) { cs *= 0.707f; }
        if (IsKeyDown(KEY_LEFT_CONTROL)) { cs *= sprint_multiplier; }

        // sorry kimiz :(
        if (w) { position.x += cs * dt * xm; position.z += cs * dt * ym; }
        if (s) { position.x += cs * dt * -xm; position.z += cs * dt * -ym; }
        if (a) { position.x += cs * dt * ym; position.z += cs * dt * -xm; }
        if (d) { position.x += cs * dt * -ym; position.z += cs * dt * xm; }

        // Creative and survival movement (vertical)
        if (debug.fly_mode) {
            if (IsKeyDown(KEY_SPACE))
                position.y += 30.0f * dt;
        } else {
            if (IsKeyPressed(KEY_SPACE))
                vertical_velo += 4.0f;
        }
        
        if (IsKeyDown(KEY_LEFT_SHIFT)) {
            if (debug.fly_mode) {
                position.y -= 30.0f * dt;
            }
            else {
                if (vertical_velo > 0.0f)
                    vertical_velo *= 0.9f;
            }
        }

        // HOTBAR
        const float scroll = floor(GetMouseWheelMove());
        
        if (scroll > 0.0f) {
            --hotbar_selected;
            if (hotbar_selected < 0) { hotbar_selected = 8; }
        } else if (scroll < 0.0f) {
            ++hotbar_selected;
            if (hotbar_selected > 8) { hotbar_selected = 0; }
        }
        

        // Gravity
        if (!debug.fly_mode) {
            const int ix = (int)(position.x);
            const int iy = (int)(position.z);

            vertical_velo += gravity * dt;
            position.y += vertical_velo * dt * 2;

            if (0.0f >= position.y) {
                position.y = 0.0f;
                vertical_velo = 0.0f;
            }
        }

        // CAMERA
        camera.position = (Vector3){ position.x, position.y + player_height, position.z };

        Vector2 md = GetMouseDelta();
        look.x += md.x / sensitivity;
        look.y += md.y / sensitivity;

        look.x = fmodf(look.x, PI * 2);
        look.y = clamp(look.y, -1.57f, 1.57f);

        Vector3 CP = camera.position;
        camera.target = (Vector3){ CP.x + cos(look.x), CP.y - tan(look.y), CP.z + sin(look.x) };




        // CHUNKS
        vec2i16_t new_chunk_pos = (vec2i16_t){ floor(position.x / chunk_size.x), floor(position.z / chunk_size.z) };

        if (!vec2i16_t_equals(new_chunk_pos, current_chunk_pos) && !chunk_thread_running && debug.terrain_loading) {
            // Unload chunks and load new ones on another thread, then update chunk pos
            *lpArgPtr = new_chunk_pos;
            chunk_h_thread = CreateThread(NULL, 0, update_chunks, lpArgPtr, 0, &chunk_dw_thread_id);

            current_chunk_pos = new_chunk_pos;
        }

        // DRAW
        BeginDrawing();
        ClearBackground(debug.test_environment ? NIGHT : SKYBLUE);

        // 3D
        BeginMode3D(camera);

        // Chunk (put in function later)
        for (int x = -32; x < 32; ++x) {
            for (int y = 0; y < chunk_size.y; ++y) {
                for (int z = 0; z < 1; ++z) {
                    block_t block = get_block((vec3i32_t) { x, y, z });

                    if (block.type != 1)
                        place_cube(x, y, z, block);
                }
            }
        }

        // DEBUG STUFF
        if (debug.test_environment) {   // Gizmos
            DrawGrid(8, 16.0f);
            DrawLine3D((Vector3) { 0.0f, position.y, 0.0f }, (Vector3) { 8.0f, position.y, 0.0f }, RED);
            DrawLine3D((Vector3) { 0.0f, position.y, 0.0f }, (Vector3) { 0.0f, position.y, 8.0f }, BLUE);
        }

        if (debug.show_chunk_borders) { // Chunk borders
            for (int16_t x = -1; x <= 2; ++x) {
                for (int16_t y = -1; y <= 2; ++y) {
                    vec2i16_t local = (vec2i16_t){ (current_chunk_pos.x + x) * chunk_size.x, (current_chunk_pos.y + y) * chunk_size.z };
                    DrawLine3D((Vector3) { local.x, 0, local.y }, (Vector3) { local.x, chunk_size.y, local.y }, RED);
                }
            }
        }

        DrawModel(model, (Vector3) { 0.0f, 0.0f, 0.0f }, 16.0f, WHITE);

        EndMode3D();

        // Water screen effect (under ui ofc)
        if (position.y + player_height < (float)sea_level - 0.2f && !debug.test_environment) {
            DrawRectangle(0, 0, window_width, window_height, block_colors[BLOCK_WATER]);
        }

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
        // Info Strings
        DrawText("Minecrate v0.2 [CHUNK UPDATE]", 10, 10, 30, WHITE);

        if (debug.display_info) {
            uint16_t tot_loaded_chunks = get_total_loaded_chunks();

            char chunk_num_s[64];
            snprintf(chunk_num_s, 63, "Loaded Chunks: %d / %d avaliable (%.01f%%)", tot_loaded_chunks, num_chunks, ((float)tot_loaded_chunks / (float)num_chunks) * 100.0f);

            char memory_s[64];
            snprintf(memory_s, 63, "Total memory usage: %.01f MB  (%d bytes)", chunk_mem_usage / 1000000.0f, chunk_mem_usage);

            char render_dist_s[32];
            snprintf(render_dist_s, 31, "Render distance: %d", render_distance);

            char position_string[64];
            snprintf(position_string, 63, "Position: %d, %d, %d", (int)floor(position.x), (int)floor(position.y), (int)floor(position.z));

            char c_position_string[64];
            snprintf(c_position_string, 63, "Chunk Position: %d, %d", new_chunk_pos.x, new_chunk_pos.y);


            // Draw
            DrawText("-- performance --", 10, 50, 20, INFO_TITLE_COL);
            DrawText(fps_string, 10, 70, 20, INFO_COL);

            DrawText("-- player --", 10, 110, 20, INFO_TITLE_COL);
            DrawText(position_string, 10, 130, 20, INFO_COL);
            DrawText(c_position_string, 10, 150, 20, INFO_COL);

            DrawText("-- terrain --", 10, 190, 20, INFO_TITLE_COL);
            DrawText(render_dist_s, 10, 210, 20, INFO_COL);
            DrawText(chunk_num_s, 10, 230, 20, INFO_COL);

            DrawText("-- memory --", 10, 270, 20, INFO_TITLE_COL);
            DrawText(memory_s, 10, 290, 20, INFO_COL);

            if (chunk_thread_running)
                DrawText("Updating chunks...", 10, window_height - 40, 30, RED);
        }

        EndDrawing();
    }

    // Free memory and close.
    free_chunks();
    CloseWindow();

    return 0;
}

void place_cube(int x, int y, int z, block_t block) {
    Color col = block_colors[block.type];

    DrawCube((Vector3) { x + 0.5f, y + 0.5f, z + 0.5f }, 1.0f, 1.0f, 1.0f, (Color) {
        clampint(col.r + rand() / 128 / 32, 0, 255),
        clampint(col.g + rand() / 128 / 32, 0, 255),
        clampint(col.b + rand() / 128 / 32, 0, 255),
        255
    });
}

DWORD WINAPI update_chunks(LPVOID lpParameter) {
    vec2i16_t new_chunk_pos;
    new_chunk_pos = *(vec2i16_t *)lpParameter;

    unload_bounds(new_chunk_pos);
    load_bounds(new_chunk_pos);

    printf("\nLoaded chunks succesfully....");

    return 0;
}

_Bool is_thread_running(HANDLE hThread) {
    DWORD result = WaitForSingleObject(hThread, 0);
    return !result == WAIT_OBJECT_0;
}