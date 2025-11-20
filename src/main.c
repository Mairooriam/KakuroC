#include "kakuro.h"
#include "raylib.h"

#include <stdio.h>
#include <string.h>
int main(void) {
  // Initialize the window
  InitWindow(800, 600, "Raylib Hello World");
  size_t size = 50;
  arr_Nodes grid = {0};
  grid.capacity = 255;
  grid.nodes = malloc(sizeof(Node *) * grid.capacity);

  size_t x_dimension = 11;
  size_t y_dimension = 11;

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
  while (!WindowShouldClose()) {

    // INPUT
    if (IsKeyDown(KEY_RIGHT)) {
      printf("[KEY] - right\n");
      if (cursor->pos.x < x_dimension - 1) {
        cursor->pos.x++;
        update = true;
      }
    }
    if (IsKeyDown(KEY_LEFT)) {
      printf("[kKEY] - left\n");
      if (cursor->pos.x > 0) {
        cursor->pos.x--;
        update = true;
      }
    }
    if (IsKeyDown(KEY_UP)) {
      printf("[KEY] - up\n");
      if (cursor->pos.y > 0) {
        cursor->pos.y--;
        update = true;
      }
    }
    if (IsKeyDown(KEY_DOWN)) {
      printf("[KEY] - down\n");
      if (cursor->pos.y < y_dimension - 1) {
        cursor->pos.y++;
        update = true;
      }
    }

    // UPDATE
    if (update) {
      size_t index = cursor->pos.y * y_dimension + cursor->pos.x;
      Node *node = grid.nodes[index];
      node->type = TILETYPE_CLUE;
      printf("Tile at index %zu (%i,%i)updated to clue. ", index, cursor->pos.x,
             cursor->pos.y);

      char buf[1024];
      node_to_string(buf, sizeof buf, node);
      printf("node: %s", buf);
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
