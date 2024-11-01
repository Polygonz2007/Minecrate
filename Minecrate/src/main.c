
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
#include "block.h" // tres mimires
#include "chunk.h" // finalmente mimir
#include "mesh.h" // dos mimires
#include "texture.h"


// COLORS
#define INFO_TITLE_COL (Color){ 255, 255, 255, 215 }
#define INFO_COL (Color){ 255, 255, 255, 175 }
#define INFO_COL_DARK (Color){ 0, 0, 0, 175 }
#define NIGHT (Color) { 0, 20, 80, 255 }

// DEBUG COLORS
#define DEBUG_RED (Color) { 255, 0, 0, 127 }
#define DEBUG_GREEN (Color) { 0, 255, 0, 127 }
#define DEBUG_BLUE (Color) { 0, 0, 255, 127 }

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
DWORD WINAPI update_chunks(LPVOID lpParameter);
_Bool is_thread_running(HANDLE hThread);



// DEBUG
struct debug_settings {
    _Bool terrain_loading;
    _Bool display_info;
    _Bool fly_mode;
    _Bool show_chunk_borders;
    uint8_t chunk_border_range;
};

struct debug_settings debug = {
    .terrain_loading = true,
    .display_info = true,
    .fly_mode = false,
    .show_chunk_borders = true,
    .chunk_border_range = 2
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
    SetTargetFPS(240);
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    SetConfigFlags(FLAG_VSYNC_HINT);

    // CAMERA
    Camera camera = { 0 };
    camera.position = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 90.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    float sensitivity = 300.0f; // Higher sensitivity --> Less rotation per px moved

    // PHYSICS
    const float gravity = -21.0f;
    _Bool on_ground = false;


    // PLAYER SETTINGS AND DATA
    Vector3 player_position = { 0.5f, 0.0f, 0.5f }; // player_position
    Vector3 player_velocity = { 0, 0, 0 };
    vec3i32_t int_pos = { 0, 0, 0 }; // Floored pos

    Vector2 look = { 0.0f, 0.0f }; // camera rotation in radians

    float player_height = 1.77f; // y offset of camera relative to player pos
    float player_speed = 8.0f;//3.8f; // unit / second
    float player_jump_power = 8.0f;
    float player_sprint_multiplier = 2.0f;


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
        init_mesh_gen();

        init_texture_atlas();
        generate_texture_atlas();

        // Load starting chunks, on different thread
        chunk_h_thread = CreateThread(NULL, 0, update_chunks, lpArgPtr, 0, &chunk_dw_thread_id);
    }

    // Performance
    _Bool start_loading_finished = false;
    const long start_time = time(NULL);




    // Main game loop
    while (!WindowShouldClose())
    {
        //
        // Window and Keys
        if (IsKeyPressed(KEY_F3))
            debug.display_info = !debug.display_info;

        if (IsKeyPressed(KEY_F4))
            debug.show_chunk_borders = !debug.show_chunk_borders;

        if (IsKeyPressed(KEY_F2))
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
        // End Window and Keys
        //


        //
        // Terrain
        _Bool chunk_thread_running = false;

        if (debug.terrain_loading)
            chunk_thread_running = is_thread_running(chunk_h_thread);

        if (!start_loading_finished && chunk_thread_running && debug.terrain_loading) {
            // Render loading screen
            BeginDrawing();
            ClearBackground(DARKBLUE);

            char s[64];
            uint16_t tot_loaded_chunks = get_total_loaded_chunks();

            float loaded = ((float)tot_loaded_chunks / (float)num_chunks);
            snprintf(s, 63, "Loading world... [%.01f%%]", loaded * 100.0f);

            DrawText(s, (window_width - MeasureText(s, 38)) * 0.5f, (window_height - 38 - 24) * 0.5f, 38, RAYWHITE);

            DrawRectangle((window_width * 0.5f) - 300.0f, (window_height * 0.5f) + 24.0f, 600.0f, 24, BLACK);
            DrawRectangle((window_width * 0.5f) - 300.0f, (window_height * 0.5f) + 24.0f, loaded * 600.0f, 24, RED);

            EndDrawing();

            continue;
        } else {
            start_loading_finished = true;
        }

        // End Terrain
        //



        //
        // Performance and Debugging
        srand(start_time);
        const float dt = GetFrameTime();

        // My beloved FPS string
        const int fps = GetFPS();
        char fps_string[12];
        snprintf(fps_string, 11, "fps: %d", fps);


        // MOVE TO ANOTHER FILE
        // MOVEMENT
        const float xm = cos(look.x), ym = sin(look.x);
        float cs = player_speed; // Current player_speed, for multipliers and stuff

        bool w, a, s, d;
        w = IsKeyDown(KEY_W);
        a = IsKeyDown(KEY_A);
        s = IsKeyDown(KEY_S);
        d = IsKeyDown(KEY_D);

        if ((w && a) || (w && d) || (s && a) || (s && d)) { cs *= 0.707f; }
        if (IsKeyDown(KEY_LEFT_CONTROL)) { cs *= player_sprint_multiplier; }

        // sorry kimiz :(
        if (w) { player_position.x += cs * dt * xm; player_position.z += cs * dt * ym; }
        if (s) { player_position.x += cs * dt * -xm; player_position.z += cs * dt * -ym; }
        if (a) { player_position.x += cs * dt * ym; player_position.z += cs * dt * -xm; }
        if (d) { player_position.x += cs * dt * -ym; player_position.z += cs * dt * xm; }

        int_pos = (vec3i32_t){ player_position.x, player_position.y, player_position.z };
        if (player_position.x < 0)
            int_pos.x--;
        if (player_position.z < 0)
            int_pos.z--;

        // Creative and survival movement (vertical)
        if (debug.fly_mode) {
            if (IsKeyDown(KEY_SPACE))
                player_position.y += 30.0f * dt;
        } else {
            if (IsKeyPressed(KEY_SPACE))
                player_velocity.y += player_jump_power;
        }
        
        if (IsKeyDown(KEY_LEFT_SHIFT)) {
            if (debug.fly_mode) {
                player_position.y -= 30.0f * dt;
            }
            else {
                if (player_velocity.y > 0.0f)
                    player_velocity.y *= 0.9f;
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
            const int ix = (int)(player_position.x);
            const int iy = (int)(player_position.z);

            player_velocity.y += gravity * dt;
            player_position.y += player_velocity.y * dt;

            block_t standing_on_block = get_block((vec3i32_t){int_pos.x, int_pos.y - 1, int_pos.z});
            block_t inside_block = get_block((vec3i32_t) { int_pos.x, int_pos.y, int_pos.z });
            
            // until i add collision we just teleport up until were not inside block
            _Bool in_block = !(inside_block.type == BLOCK_AIR || inside_block.type == BLOCK_WATER || inside_block.type == BLOCK_UNDEFINED);
            _Bool standing_block = !(standing_on_block.type == BLOCK_AIR || standing_on_block.type == BLOCK_WATER || standing_on_block.type == BLOCK_UNDEFINED);
            
            if (standing_block && player_position.y < int_pos.y) {
                player_velocity.y = 0;
                player_position.y = (float)int_pos.y;
            }

            if (in_block)
                player_position.y++;
            
        }


        // CAMERA
        camera.position = (Vector3){ player_position.x, player_position.y + player_height, player_position.z };

        Vector2 md = GetMouseDelta();
        look.x += md.x / sensitivity;
        look.y += md.y / sensitivity;

        look.x = fmodf(look.x, PI * 2);
        look.y = clamp(look.y, -1.57f, 1.57f);

        Vector3 CP = camera.position;
        camera.target = (Vector3){ CP.x + cos(look.x), CP.y - tan(look.y), CP.z + sin(look.x) };




        // CHUNKS
        vec2i16_t new_chunk_pos = (vec2i16_t){ floor(player_position.x / chunk_size.x), floor(player_position.z / chunk_size.z) };

        if (!vec2i16_t_equals(new_chunk_pos, current_chunk_pos) && !chunk_thread_running && debug.terrain_loading) {
            // Load new ones on another thread, then update chunk pos
            *lpArgPtr = new_chunk_pos;
            chunk_h_thread = CreateThread(NULL, 0, update_chunks, lpArgPtr, 0, &chunk_dw_thread_id);

            current_chunk_pos = new_chunk_pos;
        }

        // DRAW
        BeginDrawing();
        ClearBackground(SKYBLUE);

        // 3D
        BeginMode3D(camera);

        if (debug.show_chunk_borders) { // Chunk borders
            int8_t s = debug.chunk_border_range;

            for (int16_t x = -s; x <= s + 1; ++x) {
                for (int16_t y = -s; y <= s + 1; ++y) {
                    vec2i16_t local = (vec2i16_t){ (current_chunk_pos.x + x) * chunk_size.x, (current_chunk_pos.y + y) * chunk_size.z };
                    
                    //float ph = position.y;
                    if (x != s + 1)
                        DrawLine3D((Vector3) { local.x, 0, local.y }, (Vector3) { local.x + 16.0f, 0, local.y }, DEBUG_RED);

                    DrawLine3D((Vector3) { local.x, 0, local.y }, (Vector3) { local.x, chunk_size.y, local.y }, DEBUG_GREEN);

                    if (y != s + 1)
                        DrawLine3D((Vector3) { local.x, 0, local.y }, (Vector3) { local.x, 0, local.y + 16.0f }, DEBUG_BLUE);
                }
            }
        }

        // Load models from meshes (has to be done on main thread)
        // We find closest to load first (should be optimized)
        for (uint16_t i = 0; i < num_chunks; ++i) {
            int did = load_chunk_model(chunk_locs[i]);
            if (did == 0)
                break;
        }

        // Draw chunk models
        for (uint16_t i = 0; i < num_chunks; ++i) {
            if (chunk_status[i] == CHUNK_LOADED_MODEL) {
                vec2i16_t loc = chunk_locs[i];
                Vector3 cpos = (Vector3){ (float)loc.x * chunk_size.x, 0.0f, (float)loc.y * chunk_size.z };

                // Draw it
                Model c_chunk_model = chunk_models[i];
                DrawModel(c_chunk_model, cpos, 1.0f, WHITE);
            }
        }

        // Test
        DrawPlane((Vector3){player_position.x, sea_level - 0.2f, player_position.z}, (Vector2) { 1024, 1024 }, DEBUG_BLUE);

        EndMode3D();

        DrawTextureEx(texture_atlas, (Vector2) { window_width - 96 * 2, 0 }, 0.0f, 2.0f, WHITE);

        // Water screen effect (under ui ofc)
        if (get_block((vec3i32_t){ int_pos.x, player_position.y + player_height + 0.2f, int_pos.z }).type == BLOCK_WATER) {
            DrawRectangle(0, 0, window_width, window_height, DEBUG_BLUE);
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
        if (debug.display_info) {
            uint16_t tot_loaded_chunks = get_total_loaded_chunks();

            char chunk_num_s[64];
            char memory_s[64];
            char render_dist_s[32];
            char position_string[64];
            char c_position_string[64];

            snprintf(chunk_num_s, 63, "Loaded Chunks: %d / %d avaliable (%.01f%%)", tot_loaded_chunks, num_chunks, ((float)tot_loaded_chunks / (float)num_chunks) * 100.0f);
            snprintf(memory_s, 63, "Total memory usage: %.01f MB  (%lu bytes)", (chunk_memory_usage + mesh_memory_usage) * 0.000001f, chunk_memory_usage + mesh_memory_usage);
            snprintf(render_dist_s, 31, "Render distance: %d", render_distance);
            snprintf(position_string, 63, "Position: %d, %d, %d", int_pos.x, int_pos.y, int_pos.z);
            snprintf(c_position_string, 63, "Chunk Position: %d, %d", new_chunk_pos.x, new_chunk_pos.y);


            // Draw
            //DrawRectangle(0, 0, 400, 340, INFO_COL_DARK);

            DrawText("Minecrate v0.3 [MESH UPDATE]", 10, 10, 30, WHITE);
            DrawText("-- performance --", 10, 50, 20, INFO_TITLE_COL);
            DrawText(fps_string, 10, 70, 20, INFO_COL);

            DrawText("-- player --", 10, 110, 20, INFO_TITLE_COL);
            DrawText(position_string, 10, 130, 20, INFO_COL);
            DrawText(c_position_string, 10, 150, 20, INFO_COL);

            DrawText("-- terrain --", 10, 190, 20, INFO_TITLE_COL);
            DrawText(render_dist_s, 10, 210, 20, INFO_COL);
            DrawText(chunk_num_s, 10, 250, 20, INFO_COL);

            DrawText("-- memory --", 10, 290, 20, INFO_TITLE_COL);
            DrawText(memory_s, 10, 310, 20, INFO_COL);

            if (chunk_thread_running)
                DrawText("Updating chunks...", 10, window_height - 40, 30, RED);
        }

        EndDrawing();
    }

    // Free memory and close.
    free_mesh_gen();
    free_chunks();
    free_texture_atlas();

    CloseWindow();

    return 0;
}

DWORD WINAPI update_chunks(LPVOID lpParameter) {
    vec2i16_t new_chunk_pos;
    new_chunk_pos = *(vec2i16_t *)lpParameter;

    // Load chunk data
    printf("\n\nUpdating chunks.");
    double start_time = GetTime();

    unload_bounds(new_chunk_pos);
    load_bounds(new_chunk_pos);

    printf("\nUpdated chunks in %.03f seconds.", GetTime() - start_time);

    return 0;
}

_Bool is_thread_running(HANDLE hThread) {
    DWORD result = WaitForSingleObject(hThread, 0);
    return !result == WAIT_OBJECT_0;
}