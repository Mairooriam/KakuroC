#include "grid_utils.h"
#include "kakuro.h"
#include "nob.h"
#include <stdarg.h>
#include <stdlib.h>
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

int filter_non_empty_count(const Node *node, void *userdata) {
  FilterData_count *data = (FilterData_count *)userdata;
  if (!data)
    return -1;

  if (data->targetCount == data->count) {
    nob_log(NOB_INFO, "Filter targetCount = count");
    data->count = 0;
    return 1;
  }

  if (!node) // null node. reset filter. "edge of map"?
  {
    data->count = 0;
    return -1;
  }

  if (node->type == TILETYPE_EMPTY) {
    nob_log(NOB_INFO, "Filter Incrementing count++ from %zu to %zu ",
            data->count, data->count + 1);

    data->count++;
  }
  return 0;
}

FilterCb filter_cb_by_non_empty_count(size_t targetCount) {
  FilterData_count *data = malloc(sizeof(FilterData_count));
  if (!data)
    return (FilterCb){0};
  data->count = 0;
  data->targetCount = targetCount;
  return (FilterCb){.fn = filter_non_empty_count, .data = data};
}

int filter_by_tiletype(const Node *node, void *userdata) {
  FilterData_tiletype *data = (FilterData_tiletype *)userdata;
  if (!data)
    return -1;

  if (!node) {
    return -1;
  }

  if (data->tiletype_mask & (1U << node->type)) {
    return 1;
  }
  return 0;
}

// Straight from AI look for different way
uint32_t tiletype_mask(TileType first, ...) {
  uint32_t mask = 0;
  va_list args;
  va_start(args, first);

  TileType type = first;
  while (type != (TileType)-1) {
    mask |= (1U << type);
    type = va_arg(args, TileType);
  }

  va_end(args);
  return mask;
}

FilterCb filter_cb_by_tiletype(uint32_t mask) {
  FilterData_tiletype *data = malloc(sizeof(FilterData_tiletype));
  if (!data)
    return (FilterCb){0};
  data->tiletype_mask = mask;
  return (FilterCb){.fn = filter_by_tiletype, .data = data};
}

int modify_nodetype_to(Node *node, void *userdata) {
  ModifyData_tiletype *data = (ModifyData_tiletype *)userdata;
  if (!data) {
    nob_log(NOB_WARNING, "Tried to modify node with invalid data");
    return -1;
  }

  if (!node) {
    nob_log(NOB_WARNING, "Tried to modify null node");
    return -1;
  }

  nob_log(NOB_INFO, "Modifying node x:%hhu, y:%hhu", node->pos.x, node->pos.y);
  node->type = data->tiletype;
  return 1;
}

int modify_node_values(Node *node, void *userdata) {
  ModifyData_nodevalue *data = (ModifyData_nodevalue *)userdata;
  if (!data) {
    nob_log(NOB_WARNING, "Tried to modify node with invalid data");
    return -1;
  }

  int contains = arr_uint8_t_contains(node->values, data->value);
  if (contains >= 0) {
    nob_da_remove_unordered(node->values, (size_t)contains);
  }

  return 1;
}
int modify_cursor_sight(Node *node, void *userdata) {
  ModifyData_cursor_sight *data = (ModifyData_cursor_sight *)userdata;
  if (!data) {
    nob_log(NOB_WARNING, "Tried to modify node with invalid data");
    return -1;
  }
  nob_da_append(data->cursor->sight, node->pos);
  return 1;
}
int Modify_node_field(Node *node, void *userdata) {
  ModifyData_nodeField *data = (ModifyData_nodeField *)userdata;
  if (!data) {
    return -1;
  }
  Node n = data->node;
  uint32_t flags = data->flags;

  // Copy only if flag is set
  if (flags & NODE_FIELD_POS) {
    node->pos = n.pos;
  }
  if (flags & NODE_FIELD_TYPE) {
    node->type = n.type;
  }
  if (flags & NODE_FIELD_VALUES) {
    assert(0 && "do this when needed");
  }
  if (flags & NODE_FIELD_CLUE_VALUES) {
    node->clue_possible_values = n.clue_possible_values;
  }
  if (flags & NODE_FIELD_ID) {
    node->id = n.id;
  }
  if (flags & NODE_FIELD_SUM_Y) {
    node->sum_y = n.sum_y;
  }
  if (flags & NODE_FIELD_SUM_X) {
    node->sum_x = n.sum_x;
  }
  if (flags & NODE_FIELD_X_COUNT) {
    node->x_empty_count = n.x_empty_count;
  }
  if (flags & NODE_FIELD_Y_COUNT) {
    node->y_empty_count = n.y_empty_count;
  }
  if (flags & NODE_FIELD_COLOR) {
    node->color = n.color;
  }

  return 1;
}
// TODO: this feels stupid make some generic helper for this??
ModifyCb modify_cb_node_color(Color color) {
  ModifyData_nodeField *data = malloc(sizeof(ModifyData_nodeField));
  if (!data)
    return (ModifyCb){0};
  data->node.color = color;
  data->flags = NODE_FIELD_COLOR;
  return (ModifyCb){.fn = Modify_node_field, .data = data};
}

ModifyCb modify_cb_node_tiletype(TileType type) {
  ModifyData_nodeField *data = malloc(sizeof(ModifyData_nodeField));
  if (!data)
    return (ModifyCb){0};
  data->node.type = type;
  data->flags = NODE_FIELD_TYPE;
  return (ModifyCb){.fn = Modify_node_field, .data = data};
}

ModifyCb modify_cb_cursor_sight(CursorNode *node) {
  ModifyData_cursor_sight *data = malloc(sizeof(ModifyData_cursor_sight));
  if (!data)
    return (ModifyCb){0};
  data->cursor = node;
  return (ModifyCb){.fn = modify_cursor_sight, .data = data};
}

ModifyCb modify_cb_node_values(uint8_t value) {
  ModifyData_nodevalue *data = malloc(sizeof(ModifyData_nodevalue));
  if (!data)
    return (ModifyCb){0};
  data->value = value;
  return (ModifyCb){.fn = modify_node_values, .data = data};
}

static void _explore_direction_stop_at_filter(arr_Nodes *grid, Node *start,
                                              Direction dir, FilterCb filter,
                                              ModifyCb modify) {
  Node *current = start;
  while (current != NULL) {
    current = _kak_get_next_node(grid, current, dir);
    // if (!current) {
    //   break;
    // } // if hitting null node it breaks filter since it doenst reset filter
    if (filter.fn(current, filter.data)) {
      break;
    } else {
      modify.fn(current, modify.data);
    }
  }
}

static void _explore_direction_do_at_filter(arr_Nodes *grid, Node *start,
                                            Direction dir, FilterCb filter,
                                            ModifyCb modify) {
  Node *current = start;
  while (current != NULL) {
    current = _kak_get_next_node(grid, current, dir);
    if (!current) {
      break;
    }

    if (filter.fn(current, filter.data)) {
      modify.fn(current, modify.data);
    }
  }
}

// Executes modify until it hits filter
int kak_explore_from_node_until(arr_Nodes *grid, Node *target, FilterCb filter,
                                ModifyCb modify) {
  nob_log(NOB_INFO, "Working on DIR_LEFT");
  _explore_direction_stop_at_filter(grid, target, DIR_LEFT, filter, modify);
  nob_log(NOB_INFO, "Working on DIR_RIGHT");
  _explore_direction_stop_at_filter(grid, target, DIR_RIGHT, filter, modify);
  nob_log(NOB_INFO, "Working on DIR_UP");
  _explore_direction_stop_at_filter(grid, target, DIR_UP, filter, modify);
  nob_log(NOB_INFO, "Working on DIR_DOWN");
  _explore_direction_stop_at_filter(grid, target, DIR_DOWN, filter, modify);
  return 1;
}

int kak_iterate_grid_do_on_filter(arr_Nodes *grid, FilterCb filter,
                                  ModifyCb modify) {
  nob_da_foreach(Node *, it, grid) {
    if (filter.fn((*it), filter.data)) {
      modify.fn((*it), modify.data);
      return 1;
    }
  }
  return -1;
}

// Executes modify on each time it hits filter
int kak_explore_from_node_on_each(arr_Nodes *grid, Node *target,
                                  FilterCb filter, ModifyCb modify) {
  _explore_direction_do_at_filter(grid, target, DIR_LEFT, filter, modify);
  _explore_direction_do_at_filter(grid, target, DIR_RIGHT, filter, modify);
  _explore_direction_do_at_filter(grid, target, DIR_UP, filter, modify);
  _explore_direction_do_at_filter(grid, target, DIR_DOWN, filter, modify);
  return 1;
}
