#include "ht.h"
#include "kakuro.h"
#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "raymath.h"
#include "rlgl.h"

int main(void) {
  // Initialize the window
  InitWindow(800, 600, "Raylib Hello World");
  SetTargetFPS(60);

  // Grid INIT
  arr_Nodes *grid = arr_nodes_create(11, 11);
  size_t bufsize = 1024 * 15;
  char buf[bufsize];
  memset(buf, 0, bufsize);
  for (size_t y = 0; y < grid->x_dimension; y++) {
    for (size_t x = 0; x < grid->y_dimension; x++) {
      Node *node = node_create_empty((Vec2u8){x, y});
      arr_nodes_add(grid, node);
    }
  }
  printf("Printing nodes: \n");
  arr_nodes_to_string(buf, bufsize, grid);
  printf("nodes: \n%s", buf);

  // Context init
  KakuroContext ctx = {0};
  ctx.Cursor_tile.tile = node_create((Vec2u8){0, 0}, TILETYPE_CURSOR, 0, 0);
  ctx.Cursor_tile.sight = arr_Vec2u8_create(100);
  ctx.state = APP_STATE_NONE;
  Camera2D camera = {0};
  camera.zoom = 1.0f;
  ctx.camera = &camera;
  ctx.margin = 5;
  ctx.size = 50;
  ctx.grid = grid;
  ctx.combination_map = ht_create();
  ctx.possible_sums_per_count = arr_uint8_t_2d_create(10);

  arr_nodes_serialize("test.txt", grid);
  // TODO: fix leak
  arr_Nodes *tmparr = arr_nodes_create(grid->x_dimension, grid->y_dimension);
  arr_nodes_deserialize("test.txt", tmparr);

  // TODO: leakinGg memory not freeing old nodes
  // Load save
  ctx.grid = arr_nodes_create(ctx.grid->x_dimension, ctx.grid->y_dimension);
  arr_nodes_deserialize("savefile.txt", ctx.grid);

  // Calculate count sum combinations
  cache_possible_sums(ctx.combination_map, ctx.possible_sums_per_count);

  // check clue tiles
  clue_tile_45_checker(ctx.grid);

  // set empty tile sums according to clues
  clue_set_all_empty_sums(ctx.grid);

  while (!WindowShouldClose()) {
    //    float delta_time = GetFrameTime(); // Get time since last frame

    // END OF INPUTS
    input_process(&ctx);
    // TODO: if need
    update_process(&ctx);

    // RENDERING
    BeginDrawing();
    BeginMode2D(camera);

    ClearBackground(RAYWHITE);
    DrawText("Hello, World!", 190, 200, 20, LIGHTGRAY);
    render_grid(ctx.grid, ctx.margin, ctx.size);
    render_node(ctx.Cursor_tile.tile, ctx.margin, ctx.size,
                (void *)&ctx.Cursor_tile);
    render_state_info(ctx.state);

    EndMode2D();

    // MOUSE REF
    DrawCircleV(GetMousePosition(), 4, DARKGRAY);
    DrawTextEx(
        GetFontDefault(), TextFormat("[%i, %i]", GetMouseX(), GetMouseY()),
        Vector2Add(GetMousePosition(), (Vector2){-44, -24}), 20, 2, BLACK);

    EndDrawing();
  }

  // CLEANUP
  CloseWindow();

  return 0;
}
