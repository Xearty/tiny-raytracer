#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif

#include "raylib.h"
#include "json.h"
#include "util.h"
#include "types.h"
#include "raytracer.h"

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <algorithm>
#include <limits>
#include <iostream>
#include <glm/glm.hpp>

using namespace glm;

int main() {
    InitWindow(1600, 900, "raytracing second attempt");
    DisableCursor();

    Xcamera camera;
    camera.pos = vec3(0.0f, 1.0f, 0.0f);
    float pitch = 0;
    float yaw = -90;
    camera_look_at(&camera, euler_angles_to_vec3(pitch, yaw));

    World world = {};
    bool world_loaded = load_world(&world, "scene.json");
    if (!world_loaded) {
        puts("Failed loading world!");
        return 1;
    }

    const float mouse_sensitivity = 0.1f;
    const float movement_speed = 0.8f;

    // setting the position of the cursor in the middle of the window
    // so the camera doesn't jump the first time you move the mouse
    SetMousePosition(GetScreenWidth() / 2, GetScreenHeight() / 2);

    while (!WindowShouldClose()) {
        if (IsKeyDown(KEY_W)) {
            camera.pos -= camera.z_basis * movement_speed;
        }
        if (IsKeyDown(KEY_S)) {
            camera.pos += camera.z_basis * movement_speed;
        }
        if (IsKeyDown(KEY_A)) {
            camera.pos += camera.x_basis * movement_speed;
        }
        if (IsKeyDown(KEY_D)) {
            camera.pos -= camera.x_basis * movement_speed;
        }
#if defined(__unix__)
        if (IsKeyDown(KEY_P)) {
            EnableCursor();
            pause();
        }
#endif

        BeginDrawing();

        for (int y = -GetScreenHeight() / 2; y <= GetScreenHeight() / 2; ++y) {
            for (int x = -GetScreenWidth() / 2; x <= GetScreenWidth() / 2; ++x) {
                vec3 viewport_point = map_screen_to_viewport(x, y, &camera);
                Xray ray = { camera.pos, viewport_point - camera.pos };
                Color pix_color = trace_ray(&world, &camera, ray, 0.0f, inf);
                draw_pixel(x, y, pix_color);
            }
        }
        
        EndDrawing();

        vec2 delta = vec2(GetMouseDelta().x, GetMouseDelta().y);

        if (delta.x != 0 || delta.y != 0) {
            pitch -= delta.y * mouse_sensitivity;
            yaw -= delta.x * mouse_sensitivity;
            clamp_euler_angles(&pitch, &yaw);
            camera_look_at(&camera, euler_angles_to_vec3(pitch, yaw));
        }
    }

    CloseWindow();

    return 0;
}
