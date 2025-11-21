#include "kakuro.h"
#include <raylib.h>
#include <stdio.h>
Node *node_create(Vec2u8 pos, TileType type, uint8_t value, uint8_t sum_x,
                  uint8_t sum_y, float size, Coloru8 color) {
  Node *node = malloc(sizeof(Node));
  if (!node)
    return NULL;

  node->pos = pos;
  node->type = type;
  node->value = value;
  node->sum_x = sum_x;
  node->sum_y = sum_y;
  node->size = size;
  node->color = color;

  return node;
}

Node *node_create_empty(Vec2u8 pos, float size) {
  return node_create(pos, TILETYPE_EMPTY, 0, 0, 0, size,
                     (Coloru8){255, 255, 255, 255});
}
Node *node_create_clue(Vec2u8 pos, uint8_t sum_x, uint8_t sum_y, float size) {
  return node_create(pos, TILETYPE_CLUE, 0, sum_x, sum_y, size,
                     (Coloru8){55, 55, 55, 255});
}

Node *node_create_blocked(Vec2u8 pos, float size) {
  return node_create(pos, TILETYPE_BLOCKED, 0, 0, 0, size,
                     (Coloru8){0, 0, 0, 255});
}

size_t node_to_string(char *buf, size_t bufsize, const Node *node) {
  size_t written = 0;

  written =
      snprintf(buf, bufsize, "[N:(%02i,%02i) C:(r%03i,g%03i,b%03i,a%03i)] \n",
               node->pos.x, node->pos.y, node->color.r, node->color.g,
               node->color.b, node->color.a);
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
  case TILETYPE_BLOCKED:
    DrawRectangle(screen_x, screen_y, n->size, n->size,
                  (Color){255, 255, 255, 255});

  case TILETYPE_EMPTY: {
    DrawRectangle(screen_x, screen_y, n->size, n->size,
                  (Color){255, 0, 0, 255});

    char buf[3];
    snprintf(buf, 2, "%i", n->value);
    DrawText(buf, screen_x + n->size / 2, screen_y + n->size / 2, 16, GRAY);
  } break;

  // TODO: add rendering for the sums
  case TILETYPE_CLUE: {
    DrawRectangle(screen_x, screen_y, n->size, n->size,
                  (Color){125, 125, 125, 255});
    DrawLine(screen_x, screen_y, screen_x + n->size, screen_y + n->size,
             Coloru8_to_raylib(n->color));

    // X SUM TEXT
    char x_sum[5];
    snprintf(x_sum, 5, "%i", n->sum_x);
    int x1_offset_left = -15;
    int y1_offset_down = 10;
    int x1 = screen_x + n->size + x1_offset_left;
    int y1 = screen_y + y1_offset_down;
    int fontsize = 14;
    DrawText(x_sum, x1, y1, fontsize, RED);

    // Y SUM TEXT
    char y_sum[5];
    snprintf(y_sum, 5, "%i", n->sum_y);
    int x2_offset_right = 10;
    int y2_offset_up = -15;
    int x2 = screen_x + x2_offset_right;
    int y2 = screen_y + n->size + y2_offset_up;
    DrawText(y_sum, x2, y2, fontsize, RED);

  } break;
  }
}
