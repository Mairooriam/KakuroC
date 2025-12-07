#include "grid_utils.h"
#include "nob.h"
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
  FilterData *filter = (FilterData *)userdata;

  if (!filter) {
    return -1;
  }

  if (filter->type == FILTER_COUNT) {
    if (filter->data.fCount.targetCount == filter->data.fCount.count) {
      filter->data.fCount.count = 0;
      return 1;
    }

    if (node->type == TILETYPE_EMPTY) {
      filter->data.fCount.count++;
    }
  }
  return 0;
}

int filter_tiletype(const Node *node, void *userdata) {
  FilterData *filter = (FilterData *)userdata;

  if (!filter) {
    return -1;
  }

  if (filter->type == FILTER_TILETYPE) {

    if (node->type == filter->data.tiletype.tiletype) {
      return 1;
    }
  }
  return 0;
}

int modify_nodetype_to(Node *node, void *userdata) {
  ModifyData *data = (ModifyData *)userdata;
  if (!data) {
    nob_log(NOB_WARNING, "Tried to modify node with invalid data");
    return -1;
  }

  if (data->type != MODIFY_TILETYPE) {
    nob_log(NOB_WARNING, "Tried to modify node with wrong data");
    return -1;
  }

  if (!node) {
    nob_log(NOB_WARNING, "Tried to modify null node");
    return -1;
  }

  nob_log(NOB_INFO, "Modifying node x:%hhu, y:%hhu", node->pos.x, node->pos.y);
  node->type = data->data.t.tiletype;
  return 1;
}

int modify_node_values(Node *node, void *userdata) {
  ModifyData *data = (ModifyData *)userdata;
  if (data->type != MODIFY_NODE_VALUES) {
    nob_log(NOB_WARNING, "Tried to modify node with invalid data");
    return -1;
  }

  int contains = arr_uint8_t_contains(node->values, data->data.value.value);
  if (contains >= 0) {
    nob_da_remove_unordered(node->values, (size_t)contains);
  }

  return 1;
}
int ModifyData_node_field(Node *node, void *userdata) {
  ModifyData *data = (ModifyData *)userdata;
  if (data->type != MODIFY_NODE_FIELD) {
    return -1;
  }
  Node n = data->data.f.node;
  uint32_t flags = data->data.f.flags;

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

static void _explore_direction_stop_at_filter(arr_Nodes *grid, Node *start,
                                              Direction dir, FilterCb filter,
                                              ModifyCb modify) {
  Node *current = start;
  while (current != NULL) {
    current = _kak_get_next_node(grid, current, dir);
    if (!current) {
      break;
    }
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
    if (current && filter.fn(current, filter.data)) {
      break;
    } else {
      modify.fn(current, modify.data);
    }
  }
}

// Executes modify until it hits filter
int kak_explore_from_node_until(arr_Nodes *grid, Node *target, FilterCb filter,
                                ModifyCb modify) {
  _explore_direction_stop_at_filter(grid, target, DIR_LEFT, filter, modify);
  _explore_direction_stop_at_filter(grid, target, DIR_RIGHT, filter, modify);
  _explore_direction_stop_at_filter(grid, target, DIR_UP, filter, modify);
  _explore_direction_stop_at_filter(grid, target, DIR_DOWN, filter, modify);
  return 1;
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
