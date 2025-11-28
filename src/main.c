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
  size_t size = 50;

  Camera2D camera = {0};
  camera.zoom = 1.0f;

  // TODO: use new create func with dimennsions -> test find 9 rows
  size_t x_dimension = 11;
  size_t y_dimension = 11;
  float wheel = 0;
  arr_Nodes *grid = arr_nodes_create(x_dimension, y_dimension);
  size_t bufsize = 1024 * 15;
  char buf[bufsize];
  memset(buf, 0, bufsize);
  printf("Printing nodes: \n");
  arr_nodes_to_string(buf, bufsize, grid);
  printf("%s nodes: \n", buf);
  for (size_t y = 0; y < x_dimension; y++) {
    for (size_t x = 0; x < y_dimension; x++) {
      Node *node = node_create_empty((Vec2u8){x, y});
      arr_nodes_add(grid, node);
    }
  }
  memset(buf, 0, bufsize);
  printf("Printing nodes: \n");
  arr_nodes_to_string(buf, bufsize, grid);
  printf("nodes: \n%s", buf);
  int margin = 5;
  bool update = true;
  Node *cursor = node_create((Vec2u8){0, 0}, TILETYPE_CURSOR, 0, 0);
  AppState state = APP_STATE_NONE;
  arr_nodes_serialize("test.txt", grid);
  // TODO: fix leak
  arr_Nodes *tmparr = arr_nodes_create(x_dimension, y_dimension);
  arr_nodes_deserialize("test.txt", tmparr);
  while (!WindowShouldClose()) {
    //    float delta_time = GetFrameTime(); // Get time since last frame

    // START OF INPUTS
    // TODO: add if held down for xxx frames -> it goes into isKeyPressed mode
    if (IsKeyPressed(KEY_RIGHT) && cursor->pos.x < x_dimension - 1) {
      cursor->pos.x++;
      printf("[MOVE] Right to (%u, %u)\n", cursor->pos.x, cursor->pos.y);
    }

    if (IsKeyPressed(KEY_LEFT) && cursor->pos.x > 0) {
      cursor->pos.x--;
      printf("[MOVE] Left to (%u, %u)\n", cursor->pos.x, cursor->pos.y);
    }
    if (IsKeyPressed(KEY_UP) && cursor->pos.y > 0) {
      cursor->pos.y--;
      printf("[MOVE] Up to (%u, %u)\n", cursor->pos.x, cursor->pos.y);
    }
    if (IsKeyPressed(KEY_DOWN) && cursor->pos.y < y_dimension - 1) {
      cursor->pos.y++;
      printf("[MOVE] Down to (%u, %u)\n", cursor->pos.x, cursor->pos.y);
    }
    // SAVING
    if (IsKeyDown(KEY_S)) {
    } else if (IsKeyReleased(KEY_S)) {
      printf("[SAVE] - saving current map\n");
      arr_nodes_serialize("savefile.txt", grid);
    }

    // LOADING
    if (IsKeyDown(KEY_L)) {
    } else if (IsKeyReleased(KEY_L)) {
      printf("[SAVE] - Loading \"savefile.txt\" map\n");
      // TODO: leakinGg memory not freeing old nodes
      grid = arr_nodes_create(grid->x_dimension, grid->y_dimension);
      arr_nodes_deserialize("savefile.txt", grid);
    }

    // PLACING CLUE
    if (IsKeyDown(KEY_C)) {
    } else if (IsKeyReleased(KEY_C)) {
      size_t index = cursor->pos.y * y_dimension + cursor->pos.x;
      Node *node = grid->nodes[index];
      node->type = TILETYPE_CLUE;
      printf("Tile at index %zu (%u,%u) updated to clue.\n", index,
             cursor->pos.x, cursor->pos.y);
    }

    // PLACING BLOCED
    if (IsKeyDown(KEY_B)) {
    } else if (IsKeyReleased(KEY_B)) {
      size_t index = cursor->pos.y * y_dimension + cursor->pos.x;
      Node *node = grid->nodes[index];
      node->type = TILETYPE_BLOCKED;
      printf("Tile at index %zu (%u,%u) updated to blockeblockedd.\n", index,
             cursor->pos.x, cursor->pos.y);
    }

    // CHANGE STATE TO X SUM
    if (IsKeyDown(KEY_X)) {
    } else if (IsKeyReleased(KEY_X)) {
      state = APP_STATE_X_SUM;
    }

    // PLACING BLOCED
    if (IsKeyDown(KEY_Y)) {
    } else if (IsKeyReleased(KEY_Y)) {
      state = APP_STATE_Y_SUM;
    }

    // CHANGE STATE
    if (IsKeyDown(KEY_N)) {
    } else if (IsKeyReleased(KEY_N)) {
      state = APP_STATE_NONE;
    }
    // CHECK TILE FOR 45 SUM
    if (IsKeyDown(KEY_T)) {
    } else if (IsKeyReleased(KEY_T)) {
      clue_tile_45_checker_single_node(grid, cursor->pos.x, cursor->pos.y);
    }
    // CHECK ALL TILES FOR 45 SUM
    if (IsKeyDown(KEY_A)) {
    } else if (IsKeyReleased(KEY_A)) {
      clue_tile_45_checker(grid);
    }

    // CHECK POSSIBLE NUMBERS FOR TILE
    if (IsKeyDown(KEY_I)) {
    } else if (IsKeyReleased(KEY_I)) {
      clue_set_all_empty_sums(grid);
    }

    // SCROLL
    // raylib [core] example - 2d camera mouse zoom

    wheel = GetMouseWheelMove();
    if (wheel != 0) {
      Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
      camera.offset = GetMousePosition();
      camera.target = mouseWorldPos;
      float scale = 0.2f * wheel;
      camera.zoom = Clamp(expf(logf(camera.zoom) + scale), 0.125f, 256.0f);
      printf("camera zoom: %f\n", camera.zoom);
    }

    // print node info
    if (IsKeyDown(KEY_P)) {
    } else if (IsKeyReleased(KEY_P)) {
      size_t index = cursor->pos.y * y_dimension + cursor->pos.x;
      Node *node = grid->nodes[index];
      size_t bufsize = 1024;
      char buf[bufsize];
      node_to_string(buf, bufsize, node);
      printf("%s", buf);
    }

    int charPressed = GetCharPressed();
    if (charPressed >= '0' && charPressed <= '9') {
      int number = charPressed - '0';
      size_t index = cursor->pos.y * y_dimension + cursor->pos.x;
      Node *node = grid->nodes[index];
      switch (node->type) {
      case TILETYPE_BLOCKED: {
      } break;
      case TILETYPE_CLUE: {
        switch (state) {
        case APP_STATE_NONE: {
        } break;
        case APP_STATE_X_SUM: {
          if (number == 0) {
            node->sum_x = 0;
          } else {
            node->sum_x += number;
          }

        } break;
        case APP_STATE_Y_SUM: {
          if (number == 0) {
            node->sum_y = 0;
          } else {
            node->sum_y += number;
          }

        } break;
        default:
          break;
        }

      } break;
      case TILETYPE_EMPTY: {
        state = APP_STATE_TYPING;
        Node *node = arr_nodes_get(grid, cursor->pos.x, cursor->pos.y);
        arr_uint8_t_add(node->values, number);
        printf("[INPUT] Added %d to tile (%u, %u)\n", number, cursor->pos.x,
               cursor->pos.y);
      } break;

      default:
        break;
      }
    }

    if (IsKeyReleased(KEY_ENTER)) {
      state = APP_STATE_NONE;
    }

    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyReleased(KEY_K)) {
      char buf[1024];
      Node *node = arr_nodes_get(grid, cursor->pos.x, cursor->pos.y);
      arr_uint8_t_to_string(buf, 1025, node->values);
      printf("(%hhu,%hhu)values: %s", cursor->pos.x, cursor->pos.y, buf);
    }

    // END OF INPUTS

    // UPDATE
    if (update) {

      // size_t index = cursor->pos.y * y_dimension + cursor->pos.x;
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
    BeginMode2D(camera);

    ClearBackground(RAYWHITE);
    DrawText("Hello, World!", 190, 200, 20, LIGHTGRAY);
    render_grid(grid, margin, size);
    render_node(cursor, margin, size);
    render_state_info(state);

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
