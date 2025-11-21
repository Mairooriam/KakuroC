#include "kakuro.h"
#include "raylib.h"

#include <stdio.h>
#include <string.h>
int main(void) {
  // Initialize the window
  InitWindow(800, 600, "Raylib Hello World");
  size_t size = 125;
  arr_Nodes grid = {0};
  grid.capacity = 255;
  grid.nodes = malloc(sizeof(Node *) * grid.capacity);

  size_t x_dimension = 4;
  size_t y_dimension = 4;

  size_t bufsize = 1024 * 8;
  char buf[bufsize];
  memset(buf, 0, bufsize);
  printf("Printing nodes: \n");
  arr_nodes_to_string(buf, bufsize, &grid);
  printf("%s nodes: \n", buf);
  for (size_t y = 0; y < x_dimension; y++) {
    for (size_t x = 0; x < y_dimension; x++) {
      Node *node = node_create_empty((Vec2u8){x, y}, size);
      arr_nodes_add(&grid, node);
    }
  }
  memset(buf, 0, bufsize);
  printf("Printing nodes: \n");
  arr_nodes_to_string(buf, bufsize, &grid);
  printf("nodes: \n%s", buf);
  int margin = 5;
  bool update = true;
  Node *cursor = node_create_clue((Vec2u8){0, 0}, 10, 20, size);
  float movement_timer = 0.0f;
  float movement_delay = 0.05f; // Move every 0.1 seconds when key held

  while (!WindowShouldClose()) {
    float delta_time = GetFrameTime(); // Get time since last frame

    movement_timer += delta_time;
    if (movement_timer >= movement_delay) {
      bool moved = false;
      if (IsKeyDown(KEY_RIGHT) && cursor->pos.x < x_dimension - 1) {
        cursor->pos.x++;
        moved = true;
        printf("[MOVE] Right to (%u, %u)\n", cursor->pos.x, cursor->pos.y);
      }
      if (IsKeyDown(KEY_LEFT) && cursor->pos.x > 0) {
        cursor->pos.x--;
        moved = true;
        printf("[MOVE] Left to (%u, %u)\n", cursor->pos.x, cursor->pos.y);
      }
      if (IsKeyDown(KEY_UP) && cursor->pos.y > 0) {
        cursor->pos.y--;
        moved = true;
        printf("[MOVE] Up to (%u, %u)\n", cursor->pos.x, cursor->pos.y);
      }
      if (IsKeyDown(KEY_DOWN) && cursor->pos.y < y_dimension - 1) {
        cursor->pos.y++;
        moved = true;
        printf("[MOVE] Down to (%u, %u)\n", cursor->pos.x, cursor->pos.y);
      }

      if (moved) {
        movement_timer = 0.0f;
        size_t index = cursor->pos.y * y_dimension + cursor->pos.x;
        // Node *node = grid.nodes[index];
        // node->type = TILETYPE_CLUE;
        // printf("Tile at index %zu (%u,%u) updated to clue.\n", index,
        //        cursor->pos.x, cursor->pos.y);
      }
    }

    int charPressed = GetCharPressed();
    if (charPressed >= '1' && charPressed <= '9') {
      int number = charPressed - '0';
      size_t index = cursor->pos.y * y_dimension + cursor->pos.x;
      Node *node = grid.nodes[index];
      node->value = (uint8_t)number;
      node->type = TILETYPE_EMPTY;
      printf("[INPUT] Set tile at (%u, %u) to %d\n", cursor->pos.x,
             cursor->pos.y, number);
    }
    // UPDATE
    if (update) {

      size_t index = cursor->pos.y * y_dimension + cursor->pos.x;
      // Node *node = grid.nodes[index];
      // node->type = TILETYPE_CLUE;
      // printf("Tile at index %zu (%i,%i)updated to clue. ", index,
      // cursor->pos.x,
      //        cursor->pos.y);
      //
      // char buf[1024];
      // node_to_string(buf, sizeof buf, node);
      // printf("node: %s", buf);
      //
      update = false;
    }

    // RENDERING
    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawText("Hello, World!", 190, 200, 20, LIGHTGRAY);

    render_grid(&grid, margin, x_dimension, y_dimension);
    render_node(cursor, margin);
    EndDrawing();
  }

  // CLEANUP
  CloseWindow();

  return 0;
}
