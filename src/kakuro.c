#include "kakuro.h"

#include "ht.h"
#include "raymath.h" // Required for: Lerp()
#include <assert.h>
#include <raylib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NOB_IMPLEMENTATION
#include "nob.h"
Node *node_create(Vec2u8 pos, TileType type, uint8_t sum_x, uint8_t sum_y) {
  Node *node = malloc(sizeof(Node));
  if (!node)
    return NULL;

  node->pos = pos;
  node->type = type;
  node->values = arr_uint8_t_create(9);
  node->sum_x = sum_x;
  node->sum_y = sum_y;
  node->x_empty_count = 0;
  node->y_empty_count = 0;
  return node;
}

Node *node_create_empty(Vec2u8 pos) {
  return node_create(pos, TILETYPE_EMPTY, 0, 0);
}
Node *node_create_clue(Vec2u8 pos, uint8_t sum_x, uint8_t sum_y) {
  return node_create(pos, TILETYPE_CLUE, sum_x, sum_y);
}

Node *node_create_blocked(Vec2u8 pos) {
  return node_create(pos, TILETYPE_BLOCKED, 0, 0);
}

size_t node_to_string(char *buf, size_t bufsize, const Node *n) {
  size_t written = 0;
  written += snprintf(buf + written, bufsize - written, "[NODE]\n");
  written += snprintf(buf + written, bufsize - written, "pos = %i,%i\n",
                      n->pos.x, n->pos.y);
  written += snprintf(buf + written, bufsize - written, "type = %i\n", n->type);
  written += snprintf(buf + written, bufsize - written, "value = ");
  written += arr_uint8_t_to_string(buf + written, bufsize - written, n->values);
  written += snprintf(buf + written, bufsize - written, "\n");

  written +=
      snprintf(buf + written, bufsize - written, "sum_x = %i\n", n->sum_x);
  written += snprintf(buf + written, bufsize - written, "x_empty_count = %i\n",
                      n->x_empty_count);
  written +=
      snprintf(buf + written, bufsize - written, "sum_y = %i\n", n->sum_y);
  written += snprintf(buf + written, bufsize - written, "y_empty_count = %i\n",
                      n->y_empty_count);

  return written;
}

bool arr_nodes_add(arr_Nodes *arr, Node *node) {
  if (arr->count >= arr->capacity) {
    size_t new_capacity = arr->capacity * 2;

    Node **temp = realloc(arr->items, sizeof(Node *) * new_capacity);

    if (!temp) {
      printf("Failed to reallocate nodes array\n");
      return false;
    }

    arr->items = temp;
    arr->capacity = new_capacity;
  }

  arr->items[arr->count] = node;
  arr->count++;
  return true;
}
size_t arr_nodes_to_string(char *buf, size_t bufsize, const arr_Nodes *arr) {
  size_t written = 0;
  for (size_t i = 0; i < arr->count; i++) {
    written += node_to_string(buf + written, bufsize, arr->items[i]);
  }
  return written;
}
int arr_nodes_serialize(const char *path, const arr_Nodes *arr) {
  size_t written = 0;
  size_t bufsize = 1024 * 20;
  char *buf = malloc(bufsize);
  for (size_t i = 0; i < arr->count; i++) {
    written += node_to_string(buf + written, bufsize - written, arr->items[i]);
    if (written >= bufsize) {
      // TODO: grow buffer
      printf("SERIALIZER OUT OF BUFFER FIX arr_nodes_serialize");
    }
  }

  FILE *fp = fopen(path, "w");

  if (!fp) {
    return -1;
  }
  fwrite(buf, written, 1, fp);

  free(buf);
  fclose(fp);
  return written;
}

int arr_nodes_deserialize(const char *path, arr_Nodes *n) {
  printf("running deserialze\n");
  FILE *fp = fopen(path, "r");
  if (!fp) {
    printf("files not found\n");
    return -1;
  }

  char buf[256];
  size_t i = 0;
  bool inNode = false;
  Node tmpNode = {0};
  while (fgets(buf, sizeof buf, fp) != NULL) {

    buf[strcspn(buf, "\n")] = 0;

    if (strcmp(buf, "") == 0) {
      continue;
    }

    if (strcmp(buf, "[NODE]") == 0) {
      inNode = true;
      printf("Found node at line: %zu\n", i);
      continue;
    }
    // TODO: check if missing a field currently justd trusting
    // TODO: skip empty lines?
    if (inNode) {
      if (sscanf(buf, "pos = %hhu,%hhu", &tmpNode.pos.x, &tmpNode.pos.y) == 2) {
        printf("Parsed pos: %hhu,%hhu\n", tmpNode.pos.x, tmpNode.pos.y);
      } else if (sscanf(buf, "type = %i", (int *)&tmpNode.type) == 1) {
        printf("Parsed type: %i\n", tmpNode.type);

      }

      // TODO: implement values scanning
      // else if (sscanf(buf, "value = %hhu", &tmpNode.value) == 1) {
      //   printf("Parsed value: %hhu\n", tmpNode.value);
      // }

      else if (sscanf(buf, "sum_x = %hhu", &tmpNode.sum_x) == 1) {
        printf("Parsed sum_x: %hhu\n", tmpNode.sum_x);
      } else if (sscanf(buf, "x_empty_count = %hhu", &tmpNode.x_empty_count) ==
                 1) {
        printf("Parsed x_empty_count = %hhu\n", tmpNode.x_empty_count);
      } else if (sscanf(buf, "sum_y = %hhu", &tmpNode.sum_y) == 1) {
        printf("Parsed sum_y: %hhu\n", tmpNode.sum_y);
      } else if (sscanf(buf, "y_empty_count = %hhu", &tmpNode.y_empty_count) ==
                 1) {

        printf("Parsed y_empty_count = %hhu\n", tmpNode.y_empty_count);

        Node *node = node_create(tmpNode.pos, tmpNode.type, tmpNode.sum_x,
                                 tmpNode.sum_y);
        if (node) {
          arr_nodes_add(n, node);
          printf("Added node to array\n");
        }
        inNode = false;
        tmpNode = (Node){0};
      } else if (strlen(buf) == 0) {
      } else {
        printf("Unknown line: %s\n", buf);
      }
    }

    i++;
  }

  if (feof(fp)) {
    printf("Reached end of file\n");

  } else if (ferror(fp)) {
    printf("Error reading file\n");
  }

  fclose(fp);
  return 0;
}
arr_Nodes *arr_nodes_create(size_t x_dimension, size_t y_dimension) {
  if (x_dimension == 0 || y_dimension == 0) {
    assert(0 && "Cannot make array with dimensions of 0,0");
  }
  arr_Nodes *arr = malloc(sizeof(arr_Nodes));
  if (!arr) {
    printf("Failed to allocate arr_Nodes struct\n");
    return NULL;
  }

  arr->x_dimension = x_dimension;
  arr->y_dimension = y_dimension;
  arr->count = 0;
  arr->capacity = x_dimension * y_dimension;
  arr->items = malloc(sizeof(Node *) * arr->capacity);

  if (!arr->items) {
    printf("Failed to allocate nodes array\n");
    free(arr);
    return NULL;
  }

  return arr;
}
Vec2f Vec2f_add(Vec2f v1, Vec2f v2) {
  return (Vec2f){v1.x + v2.x, v1.y + v2.y};
}
Vec2u8 Vec2u8_add(Vec2u8 v1, Vec2u8 v2) {
  return (Vec2u8){v1.x + v2.x, v1.y + v2.y};
}
Color Coloru8_to_raylib(Coloru8 c) { return (Color){c.r, c.g, c.b, c.a}; }
#ifdef ANIMATED
static size_t animation_index = 0;
static float animation_timer = 0.0f;
static float animation_delay = 0.05f; // Delay between each square (seconds)
#endif

// TODO: make new struct for grid which has dimensions maybe 2d array? or stay
// in 1d?
//
void render_grid(const arr_Nodes *arr, int margin, int size) {
#ifdef ANIMATED

  animation_timer += GetFrameTime();
  if (animation_timer >= animation_delay && animation_index < arr->size) {
    animation_index++;
    animation_timer = 0.0f;
  }
#endif
  for (size_t y = 0; y < arr->x_dimension; y++) {
    for (size_t x = 0; x < arr->y_dimension; x++) {
      size_t index = x * arr->x_dimension + y;
#ifdef ANIMATED
      if (index < animation_index) {
#endif /* ifdef ANIMATED */
        Node *n = arr->items[index];

        render_node(n, margin, size);
#ifdef ANIMATED
      }
      if (animation_index >= (x_dimension * y_dimension)) {
        animation_index = 0;
      }
#endif
    }
  }
}

// TODO: implement this for text
// raylib [text] example - rectangle bounds
void render_node(const Node *n, int margin, int size) {
  // TODO: change units into floats?
  int screen_x = (int)n->pos.x * (size + margin);
  int screen_y = (int)n->pos.y * (size + margin);
  Rectangle rect = (Rectangle){screen_x, screen_y, size, size};
  Rectangle x_text_rect =
      (Rectangle){screen_x + size / 2, screen_y, size / 2, size / 2};
  Rectangle y_text_rect =
      (Rectangle){screen_x, screen_y + size / 2, size / 2, size / 2};
  int fontSize = 18;
  // TODO: probably bad to get font on rendering each node :))))
  Font font = GetFontDefault();

  switch (n->type) {
  case TILETYPE_BLOCKED: {
    DrawRectangleRec(rect, (Color){0, 0, 0, 255});
    DrawLine(screen_x, screen_y, screen_x + size, screen_y + size, BLACK);
  } break;
  case TILETYPE_EMPTY:
  case TILETYPE_EMPTY_VALID: {
    // TODO: do this some other way
    Color color = (Color){200, 25, 25, 100};
    if (n->type == TILETYPE_EMPTY_VALID) {
      color = (Color){0, 200, 0, 100};
    }
    Rectangle r1 = (Rectangle){rect.x, rect.y, (float)size, (float)size / 3};
    Rectangle r2 = (Rectangle){rect.x, rect.y + (float)size / 3, (float)size,
                               (float)size / 3};
    Rectangle r3 = (Rectangle){rect.x, rect.y + (float)size / 3 * 2,
                               (float)size, (float)size / 3};
    DrawRectangleRec(rect, color);
    // R   G  B
    // TODO: have cache for each node for the string???
    //
    //        // 1 1 1
    // 1 1 1  // 1 1 1
    //        //
    char b1[1024] = {0}, b2[1024] = {0}, b3[1024] = {0};
    size_t w1 = 0, w2 = 0, w3 = 0;

    for (size_t i = 0; i < n->values->count; i++) {
      if (i < 3) { // First line: indices 0, 1, 2
        w1 += snprintf(b1 + w1, sizeof(b1) - w1, "%hhu", n->values->items[i]);
        if (i < 2 && i + 1 < n->values->count) {
          w1 += snprintf(b1 + w1, sizeof(b1) - w1, ",");
        }
      } else if (i < 6) { // Second line: indices 3, 4, 5
        w2 += snprintf(b2 + w2, sizeof(b2) - w2, "%hhu", n->values->items[i]);
        if (i < 5 && i + 1 < n->values->count) {
          w2 += snprintf(b2 + w2, sizeof(b2) - w2, ",");
        }
      } else if (i < 9) { // Third line: indices 6, 7, 8
        w3 += snprintf(b3 + w3, sizeof(b3) - w3, "%hhu", n->values->items[i]);
        if (i < 8 && i + 1 < n->values->count) {
          w3 += snprintf(b3 + w3, sizeof(b3) - w3, ",");
        }
      }
    }

    if (w1 > 0) {
      Vector2 r1_pos = text_calculate_position(&r1, font, fontSize, b1);
      DrawTextEx(font, b1, r1_pos, fontSize, fontSize * .1f, RAYWHITE);
    }
    if (w2 > 0) {
      Vector2 r2_pos = text_calculate_position(&r2, font, fontSize, b2);
      DrawTextEx(font, b2, r2_pos, fontSize, fontSize * .1f, RAYWHITE);
    }
    if (w3 > 0) {
      Vector2 r3_pos = text_calculate_position(&r3, font, fontSize, b3);
      DrawTextEx(font, b3, r3_pos, fontSize, fontSize * .1f, RAYWHITE);
    }

    DrawRectangleLinesEx(r1, 1.0f, ORANGE);
    DrawRectangleLinesEx(r2, 1.0f, PURPLE);
    DrawRectangleLinesEx(r3, 1.0f, PINK);
    // TODO:draw id, render possible vlaues also?
  } break;

  case TILETYPE_CLUE: {
    DrawRectangleRec(rect, (Color){125, 125, 125, 255});
    DrawLine(screen_x, screen_y, screen_x + size, screen_y + size, BLACK);

    // SUMS TO CHARS

    // raylib [text] example - words alignment
    // CALC TEXT POSITION X
    if (n->sum_x != 0) {
      char x_sum[5];
      snprintf(x_sum, 5, "%i", n->sum_x);
      Vector2 x_textSize = MeasureTextEx(font, x_sum, fontSize, fontSize * .1f);
      Vector2 x_textPos = (Vector2){
          x_text_rect.x +
              Lerp(0.0f, x_text_rect.width - x_textSize.x, ((float)1) * 0.5f),
          x_text_rect.y +
              Lerp(0.0f, x_text_rect.height - x_textSize.y, ((float)1) * 0.5f)};
      DrawTextEx(font, x_sum, x_textPos, fontSize, fontSize * .1f, RAYWHITE);
      DrawRectangleLinesEx(x_text_rect, 1.0f, RED);
    }

    // CALC TEXT POSITION Y
    if (n->sum_y) {
      char y_sum[5];
      snprintf(y_sum, 5, "%i", n->sum_y);
      Vector2 y_textSize = MeasureTextEx(font, y_sum, fontSize, fontSize * .1f);
      Vector2 y_textPos = (Vector2){
          y_text_rect.x +
              Lerp(0.0f, y_text_rect.width - y_textSize.x, ((float)1) * 0.5f),
          y_text_rect.y +
              Lerp(0.0f, y_text_rect.height - y_textSize.y, ((float)1) * 0.5f)};
      // Draws texts with bounding boxes
      DrawTextEx(font, y_sum, y_textPos, fontSize, fontSize * .1f, RAYWHITE);
      DrawRectangleLinesEx(y_text_rect, 1.0f, RED);
    }

  } break;
  case TILETYPE_CURSOR: {
    DrawRectangleRec(rect, (Color){200, 0, 200, 175});

  } break;
  }
}
typedef enum { DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT } Direction;

static Node *_kak_get_next_node(arr_Nodes *grid, Node *current,
                                Direction direction) {

  uint8_t x = current->pos.x;
  uint8_t y = current->pos.y;
  Node *node = NULL;
  switch (direction) {
  case DIR_UP: {
    node = arr_nodes_get(grid, x, y - 1);

  } break;
  case DIR_DOWN: {
    node = arr_nodes_get(grid, x, y + 1);

  } break;
  case DIR_LEFT: {
    node = arr_nodes_get(grid, x - 1, y);
  } break;
  case DIR_RIGHT: {
    node = arr_nodes_get(grid, x + 1, y);

  } break;
  default: {
    nob_log(NOB_WARNING, "????");
  }
  }

  if (node == NULL) {
    nob_log(NOB_WARNING, "Tried getting node out of bounds");
    return NULL;
  } else {
    return node;
  }
}

static int _node_filter_non_empty_count(const Node *node, void *userdata) {
  FilterData *filter = (FilterData *)userdata;

  if (!filter) {
    return -1;
  }

  if (filter->type == FILTER_COUNT) {
    if (filter->data.fCount.targetCount == filter->data.fCount.count) {
      return 1;
    }

    if (node->type == TILETYPE_EMPTY) {
      filter->data.fCount.count++;
    }
  }
  return 0;
}

static int _modify_nodetype_to(Node *node, void *userdata) {
  ModifyData *data = (ModifyData *)userdata;
  if (!data) {
    nob_log(NOB_WARNING, "Tried to modify node with invalid data");
    return -1;
  }

  if (data->type != MODIFY_TILETYPE) {
    nob_log(NOB_WARNING, "Tried to modify node with wrong data");
    return -1;
  }

  nob_log(NOB_INFO, "Modifying node x:%hhu, y:%hhu", node->pos.x, node->pos.y);
  node->type = data->data.tiletype;
  return 1;
}

int kak_explore_from_node_until(arr_Nodes *grid, Node *target,
                                NodeFilterFn filter, NodeModifyFn modify) {
  // Explores surrounding nodes to the supplied node
  // until specified tiletype is found
  // uses callback on each node

  // from target go <-- --> up and down
  Node *current = target;
  FilterData filterD = {0};
  filterD.type = FILTER_COUNT;
  filterD.data.fCount.count = 0;
  filterD.data.fCount.targetCount = 0;

  ModifyData data = {0};
  data.type = MODIFY_TILETYPE;
  data.data.tiletype = TILETYPE_BLOCKED;

  while (current != NULL) {
    current = _kak_get_next_node(grid, current, DIR_LEFT);
    if (current) {
      if (filter(current, (void *)&filterD)) {
        modify(current, (void *)&data);
        filterD.data.fCount.count = 0;
      }
    }
  }

  return 1;
}

void render_state_info(int state) {
  // pos of top left
  int x = 550;
  int fontsize = 22;
  DrawText("State", x, 0, fontsize, BLACK);
  if (state == 1) {
    DrawText("Typing x sum", x, 1 * 22 + 1, fontsize, BLACK);
  } else if (state == 2) {
    DrawText("Typing y sum", x, 1 * 22 + 1, fontsize, BLACK);
  } else if (state == 0) {
    DrawText("no state", x, 1 * 22 + 1, fontsize, BLACK);
  }
}
Node *arr_nodes_get(const arr_Nodes *arr, size_t x, size_t y) {
  if (x >= arr->x_dimension || y >= arr->y_dimension) {
    return NULL;
  }

  size_t i = y * arr->y_dimension + x;
  return arr->items[i];
}

static bool has_9_empty(arr_Nodes *n, size_t start_x, size_t start_y,
                        bool dir_x) {
  size_t count = 0;
  size_t x = start_x, y = start_y;
  while (count < 9) {
    if (dir_x)
      x++;
    else
      y++;
    Node *node = arr_nodes_get(n, x, y);

    if (node == NULL || node->type != TILETYPE_EMPTY)
      return false;
    count++;
  }
  return true;
}

// check if single node has 45 sum, if it does it changes corresponding sum.
void clue_tile_45_checker_single_node(arr_Nodes *arr, size_t x, size_t y) {
  Node *node = arr_nodes_get(arr, x, y);

  // TODO: caching for clues instead of checking
  if (node->type == TILETYPE_CLUE) {
    // check 9 right
    if (has_9_empty(arr, x, y, true)) {
      node->sum_x = 45;
      node->x_empty_count = 9;
    } else {
      printf("Tile doenst have 45 sum on x axis\n");
    }

    if (has_9_empty(arr, x, y, false)) {
      node->sum_y = 45;
      node->y_empty_count = 9;
    } else {
      printf("Tile doenst have 45 sum on y axis\n");
    }
  }
}
void clue_tile_45_checker(arr_Nodes *n) {
  for (size_t y = 0; y < n->y_dimension; y++) {
    for (size_t x = 0; x < n->x_dimension; x++) {
      clue_tile_45_checker_single_node(n, x, y);
    }
  }
}
void clue_calculate_ids(arr_Nodes *arr) {
  for (size_t y = 0; y < arr->y_dimension; y++) {
    for (size_t x = 0; x < arr->x_dimension; x++) {
    }
  }
}

void clue_set_all_empty_sums(arr_Nodes *arr) {
  // set sums
  for (size_t y = 0; y < arr->y_dimension; y++) {
    for (size_t x = 0; x < arr->x_dimension; x++) {
      Node *node = arr_nodes_get(arr, x, y);
      if (node->type == TILETYPE_CLUE) {
        printf("hello from outside while but inside if\n");
        // TODO: handle if there is more than 45 space? if needed
        size_t x_temp = x;
        size_t y_temp = y;
        size_t x_count = 0;
        size_t y_count = 0;
        // Set nodes on x axis and count empty nodes on x axis
        while (x_temp++ < arr->x_dimension - 1) {
          Node *n = arr_nodes_get(arr, x_temp, y);
          if (n->type == TILETYPE_EMPTY) {
            printf("Hello this is empty node\n");
            n->sum_x = node->sum_x;
            x_count++;
          } else {
            break;
          }
          printf("cuyrrent x:%zu y:%zu\n", x_temp, y);
        }

        // Set nodes on y axis and count empty nodes on y axis
        while (y_temp++ < arr->y_dimension - 1) {
          Node *n = arr_nodes_get(arr, x, y_temp);
          if (n->type == TILETYPE_EMPTY) {
            printf("Hello this is empty node\n");
            n->sum_y = node->sum_y;
            y_count++;
          } else {
            break;
          }
          printf("cuyrrent x:%zu y:%zu\n", x, y_temp);
        }

        printf("x_count: %zu, y_count: %zu for clue at (%zu,%zu)\n", x_count,
               y_count, x, y);

        // Update count on the corresponding nodes
        y_temp = y;
        while (y_temp++ < arr->y_dimension - 1) {
          Node *n = arr_nodes_get(arr, x, y_temp);
          if (n->type == TILETYPE_EMPTY) {
            printf("Hello this is empty node\n");
            n->y_empty_count = y_count;
            printf("Set y_empty_count = %zu for node at (%zu,%zu)\n", y_count,
                   x, y_temp);
          } else {
            break;
          }
          printf("cuyrrent x:%zu y:%zu\n", x, y_temp);
        }
        node->y_empty_count = y_count;

        x_temp = x;
        while (x_temp++ < arr->x_dimension - 1) {
          Node *n = arr_nodes_get(arr, x_temp, y);
          if (n->type == TILETYPE_EMPTY) {
            printf("Hello this is empty node\n");
            n->x_empty_count = x_count;
            printf("Set x_empty_count = %zu for node at (%zu,%zu)\n", x_count,
                   x_temp, y);
          } else {
            break;
          }
          printf("cuyrrent x:%zu y:%zu\n", x_temp, y);
        }
        node->x_empty_count = x_count;
      }
    }
  }
}

bool arr_uint8_t_add(arr_uint8_t *arr, uint8_t val) {
  if (arr->count >= arr->capacity) {
    size_t newSize = arr->capacity * 2;
    uint8_t *temp = realloc(arr->items, sizeof(uint8_t) * newSize);

    if (!temp) {
      return false;
    }

    arr->capacity = newSize;
    arr->items = temp;
  }
  arr->items[arr->count] = val;
  arr->count++;
  return true;
}
size_t arr_uint8_t_to_string(char *buf, size_t bufsize,
                             const arr_uint8_t *arr) {
  size_t written = 0;
  written += snprintf(buf + written, bufsize - written, "[");
  for (size_t i = 0; i < arr->count; i++) {
    written +=
        snprintf(buf + written, bufsize - written, "%hhu", arr->items[i]);
    if (i < arr->count - 1) {
      written += snprintf(buf + written, bufsize - written, ",");
    }
  }
  written += snprintf(buf + written, bufsize - written, "]");

  return written;
}
int arr_uint8_t_deserialize(const char *str, arr_uint8_t *nodes) {
  (void)str;
  (void)nodes;
  // TODO: implement
  // will be [1,1,1,1,1,1,1]
  // if (sscanf(buf, "type = %i") == 1) {
  // }
  return -1;
}

arr_uint8_t *arr_uint8_t_create(size_t initial_capacity) {
  arr_uint8_t *arr = malloc(sizeof(arr_uint8_t));
  if (!arr) {
    printf("Failed to allocate arr_uint8_t struct\n");
    return NULL;
  }

  arr->count = 0;
  arr->capacity = initial_capacity;
  arr->items = malloc(sizeof(uint8_t) * initial_capacity);

  if (!arr->items) {
    printf("Failed to allocate data array\n");
    free(arr);
    return NULL;
  }

  return arr;
}

size_t arr_uint8_t_sum(const arr_uint8_t *arr) {
  size_t sum = 0;
  for (size_t i = 0; i < arr->count; i++) {
    sum += arr->items[i];
  }
  return sum;
}
int arr_uint8_t_contains(const arr_uint8_t *arr, uint8_t val) {
  for (size_t i = 0; i < arr->count; i++) {
    if (arr->items[i] == val) {
      return i;
    }
  }
  return -1;
}
arr_uint8_t *
arr_uint8_t_compare_and_return_if_both_not_0(const arr_uint8_t *arr1,
                                             const arr_uint8_t *arr2) {
  if (arr1->count != arr2->count) {
    assert(0 && "only supports same size arrays :))) TODO: uppdate this func");
  }

  arr_uint8_t *t_arr = arr_uint8_t_create(10);
  for (size_t i = 0; i < arr1->count; i++) {
    if ((arr1->items[i] != 0) && (arr2->items[i] != 0)) {
      nob_da_append(t_arr, i + 1); // +1 since index 0 = 1
    }
  }
  return t_arr;
}
size_t arr_uint8_t_2d_to_string(char *buf, size_t bufsize,
                                arr_uint8_t_2d *arr) {
  size_t written = 0;
  written += snprintf(buf + written, bufsize - written, "[");
  for (size_t i = 0; i < arr->count; i++) {
    written +=
        arr_uint8_t_to_string(buf + written, bufsize - written, arr->items[i]);
    if (i < arr->count - 1) {
      written += snprintf(buf + written, bufsize - written, ",");
    }
  }
  written += snprintf(buf + written, bufsize - written, "]");
  return written;
}

Vector2 text_calculate_position(const Rectangle *rect, Font font,
                                float fontSize, char *buf) {
  Vector2 textSize = MeasureTextEx(font, buf, fontSize, fontSize * .1f);
  Vector2 textPos = (Vector2){
      rect->x + Lerp(0.0f, rect->width - textSize.x, ((float)1) * 0.5f),
      rect->y + Lerp(0.0f, rect->height - textSize.y, ((float)1) * 0.5f)};
  return textPos;
}

void input_process(KakuroContext *ctx) {
  input_cursor_tile(ctx);
  input_keys(ctx);
  input_mouse(ctx);
  input_state(ctx);
}

void input_cursor_tile(KakuroContext *ctx) {
  // START OF INPUTS
  Node *cursor = ctx->Cursor_tile;
  arr_Nodes *grid = ctx->grid;
  // TODO: add if held down for xxx frames -> it goes into isKeyPressed mode
  if (IsKeyPressed(KEY_RIGHT) && cursor->pos.x < grid->x_dimension - 1) {
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
  if (IsKeyPressed(KEY_DOWN) && cursor->pos.y < grid->y_dimension - 1) {
    cursor->pos.y++;
    printf("[MOVE] Down to (%u, %u)\n", cursor->pos.x, cursor->pos.y);
  }
}

void input_keys(KakuroContext *ctx) {
  // SAVING
  Node *cursor = ctx->Cursor_tile;
  if (IsKeyDown(KEY_S)) {
  } else if (IsKeyReleased(KEY_S)) {
    printf("[SAVE] - saving current map\n");
    arr_nodes_serialize("savefile.txt", ctx->grid);
  }

  // LOADING
  if (IsKeyDown(KEY_L)) {
  } else if (IsKeyReleased(KEY_L)) {
    printf("[SAVE] - Loading \"savefile.txt\" map\n");
    // TODO: leakinGg memory not freeing old nodes
    ctx->grid =
        arr_nodes_create(ctx->grid->x_dimension, ctx->grid->y_dimension);
    arr_nodes_deserialize("savefile.txt", ctx->grid);
  }

  // PLACING CLUE
  if (IsKeyDown(KEY_C)) {
  } else if (IsKeyReleased(KEY_C)) {
    size_t index = cursor->pos.y * ctx->grid->y_dimension + cursor->pos.x;
    Node *node = ctx->grid->items[index];
    node->type = TILETYPE_CLUE;
    printf("Tile at index %zu (%u,%u) updated to clue.\n", index, cursor->pos.x,
           cursor->pos.y);
  }

  // PLACING BLOCED
  if (IsKeyDown(KEY_B)) {
  } else if (IsKeyReleased(KEY_B)) {
    size_t index = cursor->pos.y * ctx->grid->y_dimension + cursor->pos.x;
    Node *node = ctx->grid->items[index];
    node->type = TILETYPE_BLOCKED;
    printf("Tile at index %zu (%u,%u) updated to blockeblockedd.\n", index,
           cursor->pos.x, cursor->pos.y);
  }

  // print node info
  if (IsKeyDown(KEY_P)) {
  } else if (IsKeyReleased(KEY_P)) {
    size_t index = cursor->pos.y * ctx->grid->y_dimension + cursor->pos.x;
    Node *node = ctx->grid->items[index];
    size_t bufsize = 1024;
    char buf[bufsize];
    node_to_string(buf, bufsize, node);
    printf("%s", buf);
  }
  // CHECK TILE FOR 45 SUM
  if (IsKeyDown(KEY_T)) {
  } else if (IsKeyReleased(KEY_T)) {
    clue_tile_45_checker_single_node(ctx->grid, cursor->pos.x, cursor->pos.y);
  }
  // CHECK ALL TILES FOR 45 SUM
  if (IsKeyDown(KEY_A)) {
  } else if (IsKeyReleased(KEY_A)) {
    clue_tile_45_checker(ctx->grid);
  }

  // CHECK POSSIBLE NUMBERS FOR TILE
  if (IsKeyDown(KEY_I)) {
  } else if (IsKeyReleased(KEY_I)) {
    clue_set_all_empty_sums(ctx->grid);
  }

  int charPressed = GetCharPressed();
  if (charPressed >= '0' && charPressed <= '9') {
    int number = charPressed - '0';
    size_t index = cursor->pos.y * ctx->grid->y_dimension + cursor->pos.x;
    Node *node = ctx->grid->items[index];
    switch (node->type) {
    case TILETYPE_BLOCKED: {
    } break;
    case TILETYPE_CLUE: {
      switch (ctx->state) {
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
      ctx->state = APP_STATE_TYPING;
      Node *node = arr_nodes_get(ctx->grid, cursor->pos.x, cursor->pos.y);
      if (node->values->count <= 8) {
        arr_uint8_t_add(node->values, number);
        printf("[INPUT] Added %d to tile (%u, %u)\n", number, cursor->pos.x,
               cursor->pos.y);
      }
    } break;

    default:
      break;
    }
  }
  if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyReleased(KEY_K)) {
    char buf[1024];
    Node *node = arr_nodes_get(ctx->grid, cursor->pos.x, cursor->pos.y);
    arr_uint8_t_to_string(buf, 1025, node->values);
    printf("(%hhu,%hhu)values: %s", cursor->pos.x, cursor->pos.y, buf);
  }

  if (IsKeyReleased(KEY_E)) {
    // clue_calculate_possible_values(ctx->grid, cursor->pos.x, cursor->pos.y);
    if (ctx->combination_map->count == 0) {
      cache_possible_sums(ctx->combination_map);
    } else {
      for (size_t i = 0; i < ctx->combination_map->capacity; i++) {
        uint16_t key = ctx->combination_map->entries[i].key;
        if (key != 0) {
          arr_uint8_t_2d *val =
              (arr_uint8_t_2d *)ctx->combination_map->entries[i].value;
          uint8_t count = key >> 8;
          uint8_t sum = key & 0xFF;
          printf("Key: %hu (Count: %hhu, Sum: %hhu), Combinations: %zu\n", key,
                 count, sum, val->count);
          nob_da_foreach(arr_uint8_t *, subset, val) {
            printf("  Combination: ");
            for (size_t k = 0; k < (*subset)->count; k++) {
              printf("%hhu ", (*subset)->items[k]);
            }
            printf("\n");
          }
        }
      }
      nob_log(NOB_INFO, "Already calculated sum cache just printing cache!");
    }
  }

  if (IsKeyDown(KEY_R)) {
  } else if (IsKeyReleased(KEY_R)) {
    get_possible_sums_from_cache_for_selected(ctx->combination_map, ctx);
  }

  if (IsKeyDown(KEY_W)) {
  } else if (IsKeyReleased(KEY_W)) {
    printf("Populating possible");
    populate_possible_sums_for_empty_tiles(ctx->combination_map, ctx);
    Node *locked = kak_lock_correct_tiles(ctx->grid);
    // TODO: Do i want to rework my node to point to clue and clue handle the
    // updating of values?
    if (locked) {
      // printf("Found one valid tile. Updating possible sums\n");
      // // TODO: add other types such as locked etc. isValidTile etc.
      //
      // // Start from the locked position and move left to find the clue
      // if (locked->pos.x > 0) {
      //   int current_x = (int)locked->pos.x - 1;
      //
      //   while (current_x >= 0) {
      //     Node *node =
      //         arr_nodes_get(ctx->grid, (size_t)current_x, locked->pos.y);
      //
      //     if (!node || node->type != TILETYPE_EMPTY) {
      //       // Hit a non-empty tile (clue or blocked), stop
      //       break;
      //     }
      //
      //     printf("Found empty tile at (%d, %d)\n", current_x, locked->pos.y);
      //
      //     // Check if this node contains the locked value
      //     int contains =
      //         arr_uint8_t_contains(node->values, locked->values->items[0]);
      //     if (contains >= 0) {
      //       nob_da_remove_unordered(node->values, (size_t)contains);
      //       printf("Removed value %hhu from tile at (%d, %d)\n",
      //              locked->values->items[0], current_x, locked->pos.y);
      //     }
      //
      //     current_x--;
      //   }
      // }
      //

      kak_explore_from_node_until(
          ctx->grid, locked, _node_filter_non_empty_count, _modify_nodetype_to);
    }
  }
}

void input_state(KakuroContext *ctx) {
  // CHANGE STATE TO X SUM
  if (IsKeyDown(KEY_X)) {
  } else if (IsKeyReleased(KEY_X)) {
    ctx->state = APP_STATE_X_SUM;
  }

  // PLACING BLOCED
  if (IsKeyDown(KEY_Y)) {
  } else if (IsKeyReleased(KEY_Y)) {
    ctx->state = APP_STATE_Y_SUM;
  }

  // CHANGE STATE
  if (IsKeyDown(KEY_N)) {
  } else if (IsKeyReleased(KEY_N)) {
    ctx->state = APP_STATE_NONE;
  }
  if (IsKeyReleased(KEY_ENTER)) {
    ctx->state = APP_STATE_NONE;
  }
}
void input_mouse(KakuroContext *ctx) {
  // SCROLL
  // raylib [core] example - 2d camera mouse zoom
  Camera2D *camera = ctx->camera;
  ctx->mWheel = GetMouseWheelMove();
  if (ctx->mWheel != 0) {
    Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), *camera);
    camera->offset = GetMousePosition();
    camera->target = mouseWorldPos;
    float scale = 0.2f * ctx->mWheel;
    camera->zoom = Clamp(expf(logf(camera->zoom) + scale), 0.125f, 256.0f);
    printf("camera zoom: %f\n", camera->zoom);
  }
}

void app_update(KakuroContext *ctx) {
  (void)ctx;
  printf("UPDATE NOT IMPLEMENTED");
}

void clue_calculate_possible_values(arr_Nodes *arr, size_t x, size_t y) {
  // TODO: fix leaks in this
  Node *n = arr_nodes_get(arr, x, y);
  if (n->type != TILETYPE_CLUE) {
    printf("Cannot calculate values for node that is not clue\n");
    return;
  }

  size_t x_count = n->x_empty_count;
  size_t y_count = n->y_empty_count;
  uint8_t nums[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
  size_t subsets_capacity = 128;

  // X subsets
  arr_uint8_t **x_subsets = malloc(sizeof(arr_uint8_t *) * subsets_capacity);
  size_t x_subsets_size = 0;
  size_t x_temp_sum = 0;
  for (int mask = 0; mask < (1 << 9); mask++) {
    // if mask has N_empty_count it is potential combination for the sum
    if (__builtin_popcount(mask) == (int)x_count) {
      arr_uint8_t *x_subset = arr_uint8_t_create(x_count);
      for (int i = 0; i < 9; i++) {
        if (mask & (1 << i)) { // only select i if its in the mask
          arr_uint8_t_add(x_subset, nums[i]);
          x_temp_sum += nums[i];
        }
      }
      if (x_temp_sum == n->sum_x) {
        x_subsets[x_subsets_size] = x_subset;
        x_subsets_size++;
      }
      x_temp_sum = 0;
    }
  }

  // Z subsets
  arr_uint8_t **y_subsets = malloc(sizeof(arr_uint8_t *) * subsets_capacity);
  size_t y_subsets_size = 0;
  size_t y_temp_sum = 0;
  for (int mask = 0; mask < (1 << 9); mask++) {
    // if mask has N_empty_count it is potential combination for the sum
    if (__builtin_popcount(mask) == (int)y_count) {
      arr_uint8_t *y_subset = arr_uint8_t_create(y_count);
      for (int i = 0; i < 9; i++) {
        if (mask & (1 << i)) { // only select i if its in the mask
          arr_uint8_t_add(y_subset, nums[i]);
          y_temp_sum += nums[i];
        }
      }
      if (y_temp_sum == n->sum_y) {
        y_subsets[y_subsets_size] = y_subset;
        y_subsets_size++;
      }
      y_temp_sum = 0;
    }
  }

  printf("PRINTING X subsets\n");
  for (size_t i = 0; i < x_subsets_size; i++) {
    arr_uint8_t *subset = x_subsets[i];
    size_t bufSize = 1024;
    char buf[bufSize];
    arr_uint8_t_to_string(buf, bufSize, subset);
    size_t sum = arr_uint8_t_sum(subset);
    printf("[%zu] - Subset: %s = %zu\n", i, buf, sum);
  }

  printf("PRINTING Z subsets\n");
  for (size_t i = 0; i < y_subsets_size; i++) {
    arr_uint8_t *subset = y_subsets[i];
    size_t bufSize = 1024;
    char buf[bufSize];
    arr_uint8_t_to_string(buf, bufSize, subset);
    size_t sum = arr_uint8_t_sum(subset);
    printf("[%zu] - Subset: %s = %zu\n", i, buf, sum);
  }
}

void print_binary_stdout(unsigned int number) {
  if (number >> 1) {
    print_binary_stdout(number >> 1);
  }
  putc((number & 1) ? '1' : '0', stdout);
}

void cache_possible_sums(ht *combination_map) {
  if (!combination_map) {
    nob_log(NOB_ERROR, "Map supplied is NULL");
  }

  // numbers to go trough
  uint8_t nums[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};

  // 2 is lowest possible count for sums
  for (size_t count = 2; count <= 9; count++) {

    for (int mask = 0; mask < (1 << 9); mask++) {
      // if mask has N_empty_count it is potential combination for the sum
      if (__builtin_popcount(mask) == (int)count) {
        arr_uint8_t *subset = arr_uint8_t_create(count);
        size_t sum = 0;
        for (int i = 0; i < 9; i++) {
          if (mask & (1 << i)) { // only select i if its in the mask
            arr_uint8_t_add(subset, nums[i]);
            sum += nums[i];
          } else {
            arr_uint8_t_add(subset, 0);
          }
        }
        // hash map key = count + sum
        // 9 hashmaps for each count?
        // something else?
        // one big map with count + sum key?

        // Composite key: count in high 8 bits, sum in low 8 bits
        uint16_t key = ((uint16_t)count << 8) | sum;

        // Check if (count, sum) already exists
        arr_uint8_t_2d *existing =
            (arr_uint8_t_2d *)ht_get(combination_map, key);
        if (!existing) {
          // New (count, sum): create entry
          existing = malloc(sizeof(arr_uint8_t_2d));
          existing->items = malloc(sizeof(arr_uint8_t *) * 16);
          existing->count = 0;
          existing->capacity = 16;
          ht_set(combination_map, key, (void *)existing);
        }
        nob_da_append(existing, subset);
      }
    }
  }

  printf("Combination Map Contents:\n");
  for (size_t i = 0; i < combination_map->capacity; i++) {
    uint16_t key = combination_map->entries[i].key;
    if (key != 0) {
      arr_uint8_t_2d *val = (arr_uint8_t_2d *)combination_map->entries[i].value;
      uint8_t count = key >> 8;
      uint8_t sum = key & 0xFF;
      printf("Key: %hu (Count: %hhu, Sum: %hhu), Combinations: %zu\n", key,
             count, sum, val->count);
      // Use nob_da_foreach instead of manual loop
      nob_da_foreach(arr_uint8_t *, subset, val) {
        printf("  Combination: ");
        for (size_t k = 0; k < (*subset)->count; k++) {
          printf("%hhu ", (*subset)->items[k]);
        }
        printf("\n");
      }
    }
  }
}
Node *kak_get_node_under_cursor_tile(const arr_Nodes *arr, const Node *cursor) {
  return arr_nodes_get(arr, cursor->pos.x, cursor->pos.y);
}
Node *kak_lock_correct_tiles(arr_Nodes *nodes) {
  // TODO: cache of empty nodes
  nob_da_foreach(Node *, it, nodes) {
    if ((*it)->values->count == 1) {
      (*it)->type = TILETYPE_EMPTY_VALID;
      return (*it);
    }
  }
  return NULL;
}

static arr_uint8_t_2d *
get_possible_sums_from_cache_for_tile(ht *combination_map, Node *n) {

  // GETTING AND PRINTING COMBINATIONS
  //
  // FOR X
  uint8_t count_x = n->x_empty_count;
  uint8_t sum_x = n->sum_x;
  uint16_t key_x = ((uint16_t)count_x << 8) | sum_x;
  arr_uint8_t_2d *combs_x = (arr_uint8_t_2d *)ht_get(combination_map, key_x);
  if (combs_x == NULL) {
    nob_log(NOB_WARNING,
            "Value for count_x: %hhu, sum_x: %hhu doens't exist in the cache",
            count_x, sum_x);
  } else {
    printf("Key: %hu (Count: %hhu, Sum: %hhu), Combinations: %zu\n", key_x,
           count_x, sum_x, combs_x->count);
    nob_da_foreach(arr_uint8_t *, subset, combs_x) {
      nob_da_foreach(uint8_t, it, *subset) { printf("%hhu ", (*it)); }
      printf("\n");
    }
  }

  // FOR Y
  uint8_t count_y = n->y_empty_count;
  uint8_t sum_y = n->sum_y;

  uint16_t key_y = ((uint16_t)count_y << 8) | sum_y;
  arr_uint8_t_2d *combs_y = (arr_uint8_t_2d *)ht_get(combination_map, key_y);
  if (combs_y == NULL) {
    nob_log(NOB_WARNING,
            "Value for count_y: %hhu, sum_y: %hhu doens't exist in the cache",
            count_y, sum_y);
  } else {
    printf("Key: %hu (Count: %hhu, Sum: %hhu), Combinations: %zu\n", key_y,
           count_y, sum_y, combs_y->count);
    // Use nob_da_foreach instead of manual loop
    nob_da_foreach(arr_uint8_t *, subset, combs_y) {
      nob_da_foreach(uint8_t, it, *subset) { printf("%hhu ", (*it)); }
      printf("\n");
    }
  }

  // TODO: make array comparing functions
  //  1 2 3 4 5 6 7 8 9
  //  0 0 3 4 5 6 7 8 9
  //  ^ if number is 0 add to map to numbers to not include

  // If arr size not 1 -> tile is ambigious -> if the 1 arr left size != 1 it is
  // ambigious aswell
  // TODO: remove this malloc stuff?
  arr_uint8_t_2d *arr = malloc(sizeof(arr_uint8_t_2d));
  arr->capacity = 9;
  arr->count = 0;
  arr->items = malloc(sizeof(arr_uint8_t *) * arr->capacity);

  // If either X or Y combs empty ignore loop.

  if (combs_y != NULL && combs_x != NULL) {
    for (size_t i = 0; i < combs_y->count; i++) {
      for (size_t j = 0; j < combs_x->count; j++) {
        // if combs_y 1st array
        // TODO: this leaks :) compare and return returns new array
        arr_uint8_t *temp_arr = arr_uint8_t_compare_and_return_if_both_not_0(
            combs_y->items[i], combs_x->items[j]);

        nob_da_append(arr, temp_arr);
      }
    }
    // TODO: is this shit?
  } else if (combs_x == NULL && combs_y != NULL) {
    nob_log(NOB_INFO, "combs_x null. returning combs_y");
    return combs_y;
  } else if (combs_x != NULL && combs_y == NULL) {
    return combs_x;
  }
  size_t bufsize = 1024 * 8;
  char buf[bufsize];
  arr_uint8_t_2d_to_string(buf, bufsize, arr);
  // TODO: Might overflow buffer
  nob_log(NOB_INFO, "Node at: x:%hhu,y:%hhu possible values: %s\n", n->pos.x,
          n->pos.y, buf);
  return arr;
}

void get_possible_sums_from_cache_for_selected(ht *combination_map,
                                               KakuroContext *ctx) {
  Node *n = kak_get_node_under_cursor_tile(ctx->grid, ctx->Cursor_tile);
  arr_uint8_t_2d *arr =
      get_possible_sums_from_cache_for_tile(combination_map, n);
}

void populate_possible_sums_for_empty_tiles(ht *combination_map,
                                            KakuroContext *ctx) {
  nob_da_foreach(Node *, it1, ctx->grid) {
    arr_uint8_t_2d *arr =
        get_possible_sums_from_cache_for_tile(combination_map, (*it1));
    // TODO: use map in future? somethgin esle s? // make get
    // possible sums from cache for tile that takes in the
    // node and sets the values on the first iteration the the tinhgfyu
    if ((*it1)->pos.x == 9 && (*it1)->pos.y == 9) {
      printf("lol");
    }
    if (arr != NULL) {
      arr_uint8_t *uniquearr = arr_uint8_t_create(16);
      nob_da_foreach(arr_uint8_t *, it2, arr){
          nob_da_foreach(uint8_t, it3, (*it2)){if ((*it3) == 0){continue;
    }
    if (arr_uint8_t_contains(uniquearr, (*it3)) == -1) {
      nob_da_append(uniquearr, (*it3));
    }
  }
}
// TODO: leaks if called multiple times;
// TODO: can this formatting be fixed with clang format?
// 2. Configure Clang-Format Settings
// The C_Cpp.clang_format_style setting allows you to customize how clang-format
// handles macros:
//
// Show in Settings Editor
// Key macro-related options you can add:
//
// IndentPPDirectives: BeforeHash - Indents preprocessor directives
// AlignConsecutiveMacros: Consecutive - Aligns consecutive macro definitions
// MacroBlockBegin: \"^nob_da_foreach\" - Treats your macro as a block start
// MacroBlockEnd: \"^}\" - Treats closing brace as block end
(*it1)->values = uniquearr;
// TODO: random buf
char buf[1024];
arr_uint8_t_to_string(buf, 1024, uniquearr);
if ((*it1)->pos.x == 9 && (*it1)->pos.y == 9) {
  printf("lol");
}

nob_log(NOB_INFO, "Set x:%hhu,y:%hhu values to: %s", (*it1)->pos.x,
        (*it1)->pos.y, buf);
}
}
}

void shoot_ray_to_mouse_from_cursor_tile(KakuroContext *ctx) {
  (void)ctx;
  // TODO: do tihs haha ihihih
}
