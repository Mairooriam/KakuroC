#include "kakuro.h"
#include <assert.h>
#include <raylib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
Node *node_create(Vec2u8 pos, TileType type, uint8_t value, uint8_t sum_x,
                  uint8_t sum_y, float size) {
  Node *node = malloc(sizeof(Node));
  if (!node)
    return NULL;

  node->pos = pos;
  node->type = type;
  node->value = value;
  node->sum_x = sum_x;
  node->sum_y = sum_y;
  node->size = size;
  node->x_empty_count = 0;
  node->y_empty_count = 0;
  return node;
}

Node *node_create_empty(Vec2u8 pos, float size) {
  return node_create(pos, TILETYPE_EMPTY, 0, 0, 0, size);
}
Node *node_create_clue(Vec2u8 pos, uint8_t sum_x, uint8_t sum_y, float size) {
  return node_create(pos, TILETYPE_CLUE, 0, sum_x, sum_y, size);
}

Node *node_create_blocked(Vec2u8 pos, float size) {
  return node_create(pos, TILETYPE_BLOCKED, 0, 0, 0, size);
}

size_t node_to_string(char *buf, size_t bufsize, const Node *n) {
  size_t written = 0;
  written += snprintf(buf + written, bufsize + written, "[NODE]\n");
  written += snprintf(buf + written, bufsize + written, "pos = %i,%i\n",
                      n->pos.x, n->pos.y);
  written += snprintf(buf + written, bufsize + written, "type = %i\n", n->type);
  written +=
      snprintf(buf + written, bufsize + written, "value = %i\n", n->value);
  written +=
      snprintf(buf + written, bufsize + written, "sum_x = %i\n", n->sum_x);
  written += snprintf(buf + written, bufsize + written, "x_empty_count = %i\n",
                      n->x_empty_count);
  written +=
      snprintf(buf + written, bufsize + written, "sum_y = %i\n", n->sum_y);
  written += snprintf(buf + written, bufsize + written, "y_empty_count = %i\n",
                      n->y_empty_count);

  written +=
      snprintf(buf + written, bufsize + written, "size = %f\n\n", n->size);

  return written;
}

bool arr_nodes_add(arr_Nodes *arr, Node *node) {
  if (arr->size >= arr->capacity) {
    // TODO: implement
    return false;
  }

  arr->nodes[arr->size] = node;
  arr->size++;
  return true;
}
size_t arr_nodes_to_string(char *buf, size_t bufsize, const arr_Nodes *arr) {
  size_t written = 0;
  for (size_t i = 0; i < arr->size; i++) {
    written += node_to_string(buf + written, bufsize, arr->nodes[i]);
  }
  return written;
}
int arr_nodes_serialize(const char *path, const arr_Nodes *arr) {
  size_t written = 0;
  size_t bufsize = 1024 * 20;
  char *buf = malloc(bufsize);
  for (size_t i = 0; i < arr->size; i++) {
    written += node_to_string(buf + written, bufsize - written, arr->nodes[i]);
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
      } else if (sscanf(buf, "value = %hhu", &tmpNode.value) == 1) {
        printf("Parsed value: %hhu\n", tmpNode.value);
      } else if (sscanf(buf, "sum_x = %hhu", &tmpNode.sum_x) == 1) {
        printf("Parsed sum_x: %hhu\n", tmpNode.sum_x);
      } else if (sscanf(buf, "x_empty_count = %hhu", &tmpNode.x_empty_count) ==
                 1) {
        printf("Parsed x_empty_count = %hhu\n", tmpNode.x_empty_count);
      } else if (sscanf(buf, "sum_y = %hhu", &tmpNode.sum_y) == 1) {
        printf("Parsed sum_y: %hhu\n", tmpNode.sum_y);
      } else if (sscanf(buf, "y_empty_count = %hhu", &tmpNode.y_empty_count) ==
                 1) {
        printf("Parsed y_empty_count = %hhu\n", tmpNode.y_empty_count);
      } else if (sscanf(buf, "size = %f", &tmpNode.size) == 1) {
        printf("Parsed size: %f\n", tmpNode.size);

        Node *node = node_create(tmpNode.pos, tmpNode.type, tmpNode.value,
                                 tmpNode.sum_x, tmpNode.sum_y, tmpNode.size);
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
  arr_Nodes *arr = malloc(sizeof(arr_Nodes));
  if (!arr) {
    printf("Failed to allocate arr_Nodes struct\n");
    return NULL;
  }

  arr->x_dimension = x_dimension;
  arr->y_dimension = y_dimension;
  arr->size = 0;
  arr->capacity = x_dimension * y_dimension;
  arr->nodes = malloc(sizeof(Node *) * arr->capacity);

  if (!arr->nodes) {
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
void render_grid(const arr_Nodes *arr, int margin, size_t x_dimension,
                 size_t y_dimension) {
#ifdef ANIMATED

  animation_timer += GetFrameTime();
  if (animation_timer >= animation_delay && animation_index < arr->size) {
    animation_index++;
    animation_timer = 0.0f;
  }
#endif
  for (size_t y = 0; y < x_dimension; y++) {
    for (size_t x = 0; x < y_dimension; x++) {
      size_t index = x * x_dimension + y;
#ifdef ANIMATED
      if (index < animation_index) {
#endif /* ifdef ANIMATED */
        Node *n = arr->nodes[index];

        render_node(n, margin);
#ifdef ANIMATED
      }
      if (animation_index >= (x_dimension * y_dimension)) {
        animation_index = 0;
      }
#endif
    }
  }
}
void render_node(const Node *n, int margin) {
  int screen_x = (int)n->pos.x * (n->size + margin);
  int screen_y = (int)n->pos.y * (n->size + margin);

  switch (n->type) {
  case TILETYPE_BLOCKED: {
    DrawRectangle(screen_x, screen_y, n->size, n->size, (Color){0, 0, 0, 255});
    DrawLine(screen_x, screen_y, screen_x + n->size, screen_y + n->size, BLACK);
  } break;
  case TILETYPE_EMPTY: {
    int r = n->pos.y * 50;
    int g = n->pos.x * 50;
    DrawRectangle(screen_x, screen_y, n->size, n->size, (Color){r, g, 0, 255});
    // R   G  B
    char buf[3];
    snprintf(buf, 2, "%i", n->value);
    DrawText(buf, screen_x + n->size / 2, screen_y + n->size / 2, 32, GRAY);
    // TODO:draw id, render possible vlaues also?
  } break;

  // TODO: add rendering for the sums
  case TILETYPE_CLUE: {
    DrawRectangle(screen_x, screen_y, n->size, n->size,
                  (Color){125, 125, 125, 255});
    DrawLine(screen_x, screen_y, screen_x + n->size, screen_y + n->size, BLACK);

    // TODO: proper size based text offsets

    // X SUM TEXT
    char x_sum[5];
    snprintf(x_sum, 5, "%i", n->sum_x);
    int x1_offset_left = -30;
    int y1_offset_down = 10;
    int x1 = screen_x + n->size + x1_offset_left;
    int y1 = screen_y + y1_offset_down;
    int fontsize = 32;
    DrawText(x_sum, x1, y1, fontsize, RED);

    // Y SUM TEXT
    char y_sum[5];
    snprintf(y_sum, 5, "%i", n->sum_y);
    int x2_offset_right = 10;
    int y2_offset_up = -40;
    int x2 = screen_x + x2_offset_right;
    int y2 = screen_y + n->size + y2_offset_up;
    DrawText(y_sum, x2, y2, fontsize, RED);

  } break;
  case TILETYPE_CURSOR: {
    DrawRectangle(screen_x, screen_y, n->size, n->size,
                  (Color){200, 0, 200, 175});

  } break;
  }
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
  return arr->nodes[i];
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
      }
    }
  }
}
