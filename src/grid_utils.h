#pragma once
#include "kakuro.h"
#include "raylib.h"
#include <stdint.h>
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
//
// FILTERS
//
typedef enum {
  FILTER_NO_USER_DATA,
  FILTER_COUNT,
  FILTER_TILETYPE,
} FilterType;

typedef struct {
  size_t count;
  size_t targetCount;
} FilterData_count;
int filter_non_empty_count(const Node *node, void *data);

typedef struct {
  TileType tiletype;
} FilterData_tiletype;
int filter_tiletype(const Node *node, void *data);

typedef struct {
  FilterType type;
  union {
    FilterData_count fCount;
    TileType tiletype;
  } data;
} FilterData;

//
// MODIFY
//
typedef enum {
  MODIFY_TILETYPE,
  MODIFY_NODE_VALUES,
  MODIFY_NODE_FIELD
} ModifyDataType;
typedef struct {

  TileType tiletype;
} ModifyData_tiletype;
int modify_nodetype_to(Node *node, void *userdata);

typedef struct {
  uint8_t value;
} ModifyData_nodevalue;
int modify_node_values(Node *node, void *userdata);

typedef struct {
  Node node;
  uint32_t flags;  
} ModifyData_nodeField;

#define NODE_FIELD_POS         (1 << 0)
#define NODE_FIELD_TYPE        (1 << 1)
#define NODE_FIELD_VALUES      (1 << 2)
#define NODE_FIELD_CLUE_VALUES (1 << 3)
#define NODE_FIELD_ID          (1 << 4)
#define NODE_FIELD_SUM_Y       (1 << 5)
#define NODE_FIELD_SUM_X       (1 << 6)
#define NODE_FIELD_X_COUNT     (1 << 7)
#define NODE_FIELD_Y_COUNT     (1 << 8)
#define NODE_FIELD_COLOR       (1 << 9)
int ModifyData_node_field(Node *node, void *userdata);

typedef struct {
  ModifyDataType type;
  union {
    ModifyData_tiletype t;
    ModifyData_nodevalue value;
    ModifyData_nodeField f;
  } data;
} ModifyData;
