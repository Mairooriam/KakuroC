#pragma once
#include <stdint.h>
#include <stdlib.h>
// Select map size ( 180 degree rotational symmetry about the point at the
// center of the white cells) choose player to place black squares. if there is
// sums longer than 1-9 add black cell? choose how to random it. no 1 number
// sums

// difficulty -> howm nay 2-3 sums there are.

// 9 cell sums are 45.
//
//
//
// Step 1 render grid of specified size
// setp 2 render different types of nodes
// step 3 render sum lines
// step 4 render sums on right sides of lines

// good source
// https://puzzling.stackexchange.com/questions/49927/creating-a-kakuro-puzzle-with-a-unique-solution
// https://medium.com/@sebastian.charmot/an-introduction-to-binary-integer-linear-programming-bilp-using-kakuro-puzzles-eb1a8c4c6057
typedef enum {
  STATE_KEY_NONE,
  STATE_KEY_ONE,
  STATE_KEY_TWO,
  STATE_KEY_THREE,
  STATE_KEY_FOUR,
  STATE_KEY_FIVE,
  STATE_KEY_SIX,
  STATE_KEY_SEVEN,
  STATE_KEY_EIGHT,
  STATE_KEY_NINE
} StateKeyDown;
typedef struct {
  bool isPressed;
  StateKeyDown key;
} KeyPressed;

typedef struct {
  float x;
  float y;
} Vec2f;

Vec2f Vec2f_add(Vec2f v1, Vec2f v2);

typedef struct {
  uint8_t x;
  uint8_t y;
} Vec2u8;

Vec2u8 Vec2u8_add(Vec2u8 v1, Vec2u8 v2);

// TODO: create color macros?
typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t a;
} Coloru8;
typedef struct Color Color; // for raylib
Color Coloru8_to_raylib(Coloru8 c);

typedef enum {
  TILETYPE_CURSOR,
  TILETYPE_EMPTY,  // White cell where player enters numbers
  TILETYPE_CLUE,   // Black cell with sum clues (diagonal split)
  TILETYPE_BLOCKED // Solid black cell (no clues)
} TileType;

typedef enum { APP_STATE_NONE, APP_STATE_X_SUM, APP_STATE_Y_SUM } AppState;

typedef struct {
  Vec2u8 pos; // pos of cell
  TileType type;
  uint8_t value; // value of cell
  int id;
  // TODO: make sums into vec2u8?
  // TODO: make data union and if it is type clue it has sums and if empty it
  // has values
  uint8_t sum_y;         // column
  uint8_t sum_x;         // row
  uint8_t x_empty_count; // count of empty nodes for clue row or column
  uint8_t y_empty_count;
  // TODO: split rendering stuff elsewhere? or keep here? maybe in grid?
  float size; // square size
} Node;
// TODO: add freeing of node
Node *node_create(Vec2u8 pos, TileType type, uint8_t value, uint8_t sum_x,
                  uint8_t sum_y, float size);
Node *node_create_empty(Vec2u8 pos, float size);
Node *node_create_clue(Vec2u8 pos, uint8_t sum_x, uint8_t sum_y, float size);
Node *node_create_blocked(Vec2u8 pos, float size);
size_t node_to_string(char *buf, size_t bufsize, const Node *node);

// TODO: array functions
typedef struct {
  Node **nodes;
  size_t size;
  size_t capacity;
  size_t x_dimension;
  size_t y_dimension;
} arr_Nodes;

// TODO: add freeing of array
bool arr_nodes_add(arr_Nodes *arr, Node *node);
size_t arr_nodes_to_string(char *buf, size_t bufsize, const arr_Nodes *arr);
int arr_nodes_serialize(const char *path, const arr_Nodes *nodes);
int arr_nodes_deserialize(const char *path, arr_Nodes *nodes);
arr_Nodes *arr_nodes_create(size_t x_dimension, size_t y_dimension);
Node *arr_nodes_get(const arr_Nodes *arr, size_t x, size_t y);
// KAKURO
void render_grid(const arr_Nodes *arr, int margin, size_t x_dimension,
                 size_t y_dimension);
void render_node(const Node *node, int margin);
void render_state_info(int state);

void clue_tile_45_checker(arr_Nodes *n);
void clue_tile_45_checker_single_node(arr_Nodes *arr, size_t x, size_t y);
void clue_calculate_ids(arr_Nodes *arr);
void clue_calculate_possible_values(arr_Nodes *arr, size_t x, size_t y);
void clue_set_all_empty_sums(
    arr_Nodes *arr); // TODO: implement hashset and use hash of the clue nopdes
                     // instead of iterating trough all
