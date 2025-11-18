#include "raylib.h"

int main(void) {
    // Initialize the window
    InitWindow(800, 450, "Raylib Hello World");

    // Main game loop
    while (!WindowShouldClose()) {
        // Update
        // (No updates needed for this simple example)

        // Draw
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Hello, World!", 190, 200, 20, LIGHTGRAY);
        EndDrawing();
    }

    // De-initialize
    CloseWindow();

    return 0;
}