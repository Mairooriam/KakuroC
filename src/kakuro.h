#pragma
#include <stdint.h>

// Select map size ( 180 degree rotational symmetry about the point at the
// center of the white cells) choose player to place black squares. if there is
// sums longer than 1-9 add black cell? choose how to random it. no 1 number
// sums

// difficulty -> howm nay 2-3 sums there are.

typedef struct {
  float x;
  float y;
} Vec2f;

typedef struct {
  uint8_t x;
  uint8_t y;
} Vec2_u8;

typedef struct {
  Vec2_u8 pos; // pos of cell
  TileType type;
  uint8_t value; // value of cell
  uint8_t sum_y; // column
  uint8_t sum_x; // row
} Node;

typedef struct {
  Node **nodes;
  size_t size;
  size_t capacity;
} Nodes;

typedef struct {
  Nodes grid;
  Vec2_u8 dimensions;

} Grid;

typedef enum {
  KAK_TILE_EMPTY,  // White cell where player enters numbers
  KAK_TILE_CLUE,   // Black cell with sum clues (diagonal split)
  KAK_TILE_BLOCKED // Solid black cell (no clues)
} TileType;
