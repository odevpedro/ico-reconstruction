#include "../types.h"

/*
 * Halfword grid model
 *
 * Runtime Rev.093b/Rev.094/Rev.096 tightened the picture around the hot
 * writer path that feeds the 32x32 halfword table at 0x006AB080.
 *
 * This file is intentionally conservative:
 * - it models the table contract, not the full caller chain;
 * - it keeps the confirmed row/col encoding explicit;
 * - it separates bounds, clear, and push operations so the data flow stays
 *   readable when the next runtime capture lands.
 *
 * Confirmed contract:
 *   cell = (row << 5) | col
 *   row/col are both clamped to [0, 31] by the writer path
 *   the table is indexed by a monotonic counter at 0x00633D2C
 */

#define HALFWORD_GRID_DIM 32u
#define HALFWORD_GRID_CAPACITY (HALFWORD_GRID_DIM * HALFWORD_GRID_DIM)

struct halfword_grid_state {
    u16 *cells;
    u32  count;
};

static inline int halfword_grid_in_bounds(s32 row, s32 col)
{
    return row >= 0 && row < (s32)HALFWORD_GRID_DIM &&
           col >= 0 && col < (s32)HALFWORD_GRID_DIM;
}

static inline u16 halfword_grid_encode(u32 row, u32 col)
{
    return (u16)(((row & 31u) << 5) | (col & 31u));
}

static inline u32 halfword_grid_row(u16 cell)
{
    return (u32)(cell >> 5);
}

static inline u32 halfword_grid_col(u16 cell)
{
    return (u32)(cell & 31u);
}

void halfword_grid_init(struct halfword_grid_state *grid, u16 *cells)
{
    if (grid == 0)
        return;

    grid->cells = cells;
    grid->count = 0;
}

void halfword_grid_clear(struct halfword_grid_state *grid)
{
    u32 i;

    if (grid == 0 || grid->cells == 0)
        return;

    for (i = 0; i < HALFWORD_GRID_CAPACITY; i++)
        grid->cells[i] = 0;

    grid->count = 0;
}

int halfword_grid_push(struct halfword_grid_state *grid, s32 row, s32 col)
{
    if (grid == 0 || grid->cells == 0)
        return 0;

    if (!halfword_grid_in_bounds(row, col))
        return 0;

    if (grid->count >= HALFWORD_GRID_CAPACITY)
        return 0;

    grid->cells[grid->count] = halfword_grid_encode((u32)row, (u32)col);
    grid->count++;
    return 1;
}

u32 halfword_grid_count(const struct halfword_grid_state *grid)
{
    if (grid == 0)
        return 0;
    return grid->count;
}

u16 halfword_grid_cell_at(const struct halfword_grid_state *grid, u32 index)
{
    if (grid == 0 || grid->cells == 0)
        return 0;
    if (index >= grid->count)
        return 0;
    return grid->cells[index];
}

u32 halfword_grid_cell_row(const struct halfword_grid_state *grid, u32 index)
{
    return (u32)(halfword_grid_cell_at(grid, index) >> 5);
}

u32 halfword_grid_cell_col(const struct halfword_grid_state *grid, u32 index)
{
    return (u32)(halfword_grid_cell_at(grid, index) & 31u);
}

/*
 * The next runtime session can compare observed `counter=1` / zero-write
 * buckets against this model without having to rediscover the table contract.
 * The caller-side activation problem remains separate.
 */
