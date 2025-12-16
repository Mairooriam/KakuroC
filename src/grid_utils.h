#pragma once
#include "kakuro.h"
#include "raylib.h"
#include <stdint.h>

// Got little bit overboard and spent too much time so dont want
// to just scrap it so i will keep using it :))))

// filter function signature
// -1 invalid userdata, 0 filter false, >= 1 true
typedef struct Node Node;
typedef TileType TileType;
typedef int (*NodeFilterFn)(const Node *node, void *userdata);

typedef struct {
  NodeFilterFn fn;
  void *data;
} FilterCb;

// modifies node. returns -1 if tried to modify invalid node?
typedef int (*NodeModifyFn)(Node *node, void *userdata);

typedef struct {
  NodeModifyFn fn;
  void *data;
} ModifyCb;
// Executes modify until it hits filter
int kak_explore_from_node_until(arr_Nodes *grid, Node *target, FilterCb filter,
                                ModifyCb modify);
// Executes modify on each time it hits filter
int kak_explore_from_node_on_each(arr_Nodes *grid, Node *target,
                                  FilterCb filter, ModifyCb modify);

// Executes modify on hitting filter. does for each node in grid
int kak_iterate_grid_do_on_filter(arr_Nodes *grid, FilterCb filter,
                                  ModifyCb modify);
//
// FILTERS
//
typedef struct {
  size_t count;
  size_t targetCount;
} FilterData_count;
int filter_non_empty_count(const Node *node, void *data);
FilterCb filter_cb_by_non_empty_count(size_t targetCount);

typedef struct {
  uint32_t tiletype_mask;
} FilterData_tiletype;

int filter_by_tiletype(const Node *node, void *data);
uint32_t tiletype_mask(TileType first, ...);
FilterCb filter_cb_by_tiletype(TileType tiletype);

//
// MODIFY
//
typedef struct {
  TileType tiletype;
} ModifyData_tiletype;
int modify_nodetype_to(Node *node, void *userdata);

typedef struct {
  uint8_t value;
} ModifyData_nodevalue;
int modify_node_values(Node *node, void *userdata);
typedef struct {
  CursorNode *cursor;
} ModifyData_cursor_sight;
int modify_cursor_sight(Node *node, void *userdata);
typedef struct {
  arr_uint8_t *values; // values to not be duplicated
} ModifyData_delete_duplicates;
int modify_delete_duplicates_possible_values(Node *node, void *userdata);

typedef struct {
  Node *target;
  arr_uint8_t *values_to_remove;
} ModifyData_collect_used_values;
int modify_collect_used_values(Node *node, void *userdata);

typedef struct {
  Node node;
  uint32_t flags;
} ModifyData_nodeField;

#define NODE_FIELD_POS (1 << 0)
#define NODE_FIELD_TYPE (1 << 1)
#define NODE_FIELD_VALUES (1 << 2)
#define NODE_FIELD_CLUE_VALUES (1 << 3)
#define NODE_FIELD_ID (1 << 4)
#define NODE_FIELD_SUM_Y (1 << 5)
#define NODE_FIELD_SUM_X (1 << 6)
#define NODE_FIELD_X_COUNT (1 << 7)
#define NODE_FIELD_Y_COUNT (1 << 8)
#define NODE_FIELD_COLOR (1 << 9)
int Modify_node_field(Node *node, void *userdata);

// Helpers
ModifyCb modify_cb_node_color(Color color);
ModifyCb modify_cb_node_values(uint8_t value);
ModifyCb modify_cb_cursor_sight(CursorNode *node);
ModifyCb modify_cb_node_tiletype(TileType type);
ModifyCb modify_cb_delete_duplicates_from_possible_values(arr_uint8_t *arr);
ModifyCb modify_cb_collect_used_values(Node *target);