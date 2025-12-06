#pragma once
#include "ht.h"
#include <raylib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// https://dev.to/0xog_pg/function-overloading-in-c-7nj
#define overload __attribute__((overloadable))
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
  TILETYPE_EMPTY,      // White cell where player enters numbers
  TILETYPE_CLUE,       // Black cell with sum clues (diagonal split)
  TILETYPE_BLOCKED,    // Solid black cell (no clues)
  TILETYPE_EMPTY_VALID // Valid number accroding to clues
} TileType;

typedef enum {
  APP_STATE_NONE,
  APP_STATE_X_SUM,
  APP_STATE_Y_SUM,
  APP_STATE_TYPING
} AppState;

typedef struct {
  uint8_t *items;
  size_t capacity;
  size_t count;
} arr_uint8_t;
bool arr_uint8_t_add(arr_uint8_t *arr, uint8_t val);
size_t arr_uint8_t_to_string(char *buf, size_t bufsize, const arr_uint8_t *arr);
int arr_uint8_t_serialize(const char *path, const arr_uint8_t *nodes);
int arr_uint8_t_deserialize(const char *path, arr_uint8_t *nodes);
arr_uint8_t *arr_uint8_t_create(size_t initial_capacity);
size_t arr_uint8_t_sum(const arr_uint8_t *arr);

// -1 = it doesnt contain. > -1 = index of the contains
int arr_uint8_t_contains(const arr_uint8_t *arr, uint8_t val);

arr_uint8_t *
arr_uint8_t_compare_and_return_if_both_not_0(const arr_uint8_t *arr1,
                                             const arr_uint8_t *arr2);
typedef struct {
  arr_uint8_t **items;
  size_t count;
  size_t capacity;
} arr_uint8_t_2d;
size_t arr_uint8_t_2d_to_string(char *buf, size_t bufsize, arr_uint8_t_2d *arr);

// TODO: make union for clue and empty node? to separate fields a bit
typedef struct Node {
  Vec2u8 pos; // pos of cell
  // Node *clue_x; // TODO: separate empty and clue fields
  // Node *clue_y; // TODO: separate empty and clue fields or do i evne want
  // this?
  TileType type;
  arr_uint8_t *values; // value of cell
  arr_uint8_t **clue_possible_values;
  int id;
  // TODO: make sums into vec2u8?
  // TODO: make data union and if it is type clue it has sums and if empty it
  // has values
  uint8_t sum_y;         // column
  uint8_t sum_x;         // row
  uint8_t x_empty_count; // count of empty nodes for clue row or column
  uint8_t y_empty_count;
  Color color;
} Node;
// TODO: add freeing of node
Node *node_create(Vec2u8 pos, TileType type, uint8_t sum_x, uint8_t sum_y,
                  Color color);
Node *node_create_empty(Vec2u8 pos);
Node *node_create_clue(Vec2u8 pos, uint8_t sum_x, uint8_t sum_y);
Node *node_create_blocked(Vec2u8 pos);
size_t node_to_string(char *buf, size_t bufsize, const Node *node);

// TODO: array functions
typedef struct {
  Node **items;
  size_t count;
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
void render_grid(const arr_Nodes *arr, int margin, int size);
void render_node(const Node *node, int margin, int size);
void render_state_info(int state);

void clue_tile_45_checker(arr_Nodes *n);
void clue_tile_45_checker_single_node(arr_Nodes *arr, size_t x, size_t y);
void clue_calculate_ids(arr_Nodes *arr);
void clue_calculate_possible_values(arr_Nodes *arr, size_t x, size_t y);
void clue_set_all_empty_sums(
    arr_Nodes *arr); // TODO: implement hashset and use hash of the clue nopdes
                     // instead of iterating trough all
void cache_possible_sums(ht *combination_map);
Node *kak_get_node_under_cursor_tile(const arr_Nodes *arr, const Node *cursor);

// returns count of locked tiles
Node *kak_lock_correct_tiles(arr_Nodes *nodes);

Vector2 text_calculate_position(const Rectangle *rect, Font font,
                                float fontsize, char *buf);

typedef struct {
  Node *tile;
  bool moved;
} CursorNode;

// Context
typedef struct {
  CursorNode Cursor_tile;
  arr_Nodes *grid;
  AppState state;
  float mWheel;
  Camera2D *camera;
  int margin;
  int size;
  ht *combination_map;

} KakuroContext;

// INPUT
void input_process(KakuroContext *ctx);
void input_cursor_tile(KakuroContext *ctx);
void input_keys(KakuroContext *ctx);
void input_state(KakuroContext *ctx);
void input_mouse(KakuroContext *ctx);

void update_process(KakuroContext *ctx);

void get_possible_sums_from_cache_for_selected(ht *combination_map,
                                               KakuroContext *ctx);
void populate_possible_sums_for_empty_tiles(ht *combination_map,
                                            KakuroContext *ctx);
void print_binary_stdout(unsigned int number);

void shoot_ray_to_mouse_from_cursor_tile(KakuroContext *ctx);
