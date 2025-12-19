#include "ht.h"
#include "kakuro.h"
#include "nob.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
  // Initialize the window
  InitWindow(800, 600, "Raylib Hello World");
  SetTargetFPS(60);

  // Grid INIT
  arr_Nodes *grid = arr_nodes_create(11, 11);
  size_t bufsize = 1024 * 50;
  char buf[bufsize];
  memset(buf, 0, bufsize);
  for (size_t y = 0; y < grid->x_dimension; y++) {
    for (size_t x = 0; x < grid->y_dimension; x++) {
      Node *node = node_create_empty((Vec2u8){x, y});
      arr_nodes_add(grid, node);
    }
  }
  printf("Printing nodes: \n");
  // arr_nodes_to_string(buf, bufsize, grid);
  // printf("nodes: \n%s", buf);

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
  ctx.sorted_grid = arr_node_ptrs_create(16);
  ctx.valid_count_sum_cache = ht_create();

  ctx.animation.trig_target = 0.01f;
  ctx.animation.trig_time_current = 0.0f;
  ctx.animation.time_elapsed = 0.0f;
  ctx.animation.current_step = 0;
  ctx.animation.animation_playing = false;
  ctx.animation.state = ANIM_STATE_IDLE;

  arr_nodes_serialize("test.txt", grid);
  // TODO: fix leak
  arr_Nodes *tmparr = arr_nodes_create(grid->x_dimension, grid->y_dimension);
  arr_nodes_deserialize("test.txt", tmparr);

  // TODO: leakinGg memory not freeing old nodes
  // Load save
  ctx.grid = arr_nodes_create(ctx.grid->x_dimension, ctx.grid->y_dimension);
  arr_nodes_deserialize("savefile.txt", ctx.grid);

  // Calculate count sum combinations
  cache_possible_sums(ctx.combination_map, ctx.possible_sums_per_count,
                      ctx.valid_count_sum_cache);

  // inits clues with 9 tiles;
  clue_tile_45_checker(ctx.grid);

  // set empty tile sums according to clues
  clue_set_all_empty_sums(ctx.grid);
  //
  // // fills grid with possible sums
  // populate_possible_sums_for_empty_tiles(ctx.combination_map, &ctx);
  //
  for (size_t i = 0; i < ctx.grid->count; i++) {
    Node *node = &ctx.grid->items[i];
    if (node->type == TILETYPE_EMPTY) {
      nob_da_append(ctx.sorted_grid, node);
    }
  }
  size_t bufsize2 = 1024 * 10;
  char buf2[bufsize2];
  arr_node_ptrs_to_string(buf2, bufsize2, ctx.sorted_grid);
  printf("\n\nBEFORE SORTING%s\n\n", buf2);

  // Then call:
  qsort(ctx.sorted_grid->items, ctx.sorted_grid->count, sizeof(Node *),
        node_compare_possible_count);

  memset(buf2, 0, bufsize2);
  arr_node_ptrs_to_string(buf2, bufsize2, ctx.sorted_grid);
  printf("\n\nAFTER SORTING\n\n%s", buf2);

  while (!WindowShouldClose()) {
    //    float delta_time = GetFrameTime(); // Get time since last frame

    // END OF INPUTS
    input_process(&ctx);
    update_process(&ctx);
    ctx.animation.deltatime = GetFrameTime();
    if (ctx.animation.animation_playing) {
      kakV2_animate_algorithm(&ctx);
    }
    // RENDERING
    BeginDrawing();
    BeginMode2D(camera);

    ClearBackground(RAYWHITE);
    DrawText("Hello, World!", 190, 200, 20, LIGHTGRAY);
    render_grid(ctx.grid, ctx.margin, ctx.size);
    render_sorted_grid(ctx.sorted_grid, ctx.margin, ctx.size);
    render_node(ctx.Cursor_tile.tile, ctx.margin, ctx.size,
                (void *)&ctx.Cursor_tile);
    render_state_info(ctx.state);

    EndMode2D();

    // MOUSE REF
    Vector2 mouseScreen = GetMousePosition();
    Vector2 mouseWorld =
        GetScreenToWorld2D(mouseScreen, camera); // Convert to world space
    DrawCircleV(mouseScreen, 4, DARKGRAY);
    DrawTextEx(GetFontDefault(),
               TextFormat("Screen: [%i, %i] World: [%.2f, %.2f]",
                          (int)mouseScreen.x, (int)mouseScreen.y, mouseWorld.x,
                          mouseWorld.y),
               Vector2Add(mouseScreen, (Vector2){-44, -24}), 20, 2, BLACK);
    EndDrawing();
  }

  // CLEANUP
  CloseWindow();

  return 0;
}
