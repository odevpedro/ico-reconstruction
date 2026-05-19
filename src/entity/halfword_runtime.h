#ifndef ICO_HALFWORD_RUNTIME_H
#define ICO_HALFWORD_RUNTIME_H

#include "../types.h"

#define HALFWORD_GRID_DIM 32u
#define HALFWORD_GRID_CAPACITY (HALFWORD_GRID_DIM * HALFWORD_GRID_DIM)
#define HALFWORD_WRITER_CAPACITY 1024u

struct halfword_grid_state {
    u16 *cells;
    u32  count;
};

struct halfword_writer_session {
    u16      *cells;
    u32       capacity;
    u32       count;
    u32       observed_writes;
    u32       final_counter;
    u32       world_state_raw;
    ico_ptr32 entry_callsite;
    ico_ptr32 return_pc;
};

enum halfword_writer_bucket {
    HALFWORD_BUCKET_UNKNOWN = 0,
    HALFWORD_BUCKET_EMPTY = 1,
    HALFWORD_BUCKET_SINGLE_NO_WRITE = 2,
    HALFWORD_BUCKET_SINGLE_ONE_WRITE = 3,
    HALFWORD_BUCKET_MULTI_WRITE = 4,
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

static inline int halfword_writer_in_bounds(s32 row, s32 col)
{
    return row >= 0 && row < (s32)HALFWORD_GRID_DIM &&
           col >= 0 && col < (s32)HALFWORD_GRID_DIM;
}

static inline u16 halfword_writer_encode(u32 row, u32 col)
{
    return (u16)(((row & 31u) << 5) | (col & 31u));
}

void halfword_grid_init(struct halfword_grid_state *grid, u16 *cells);
void halfword_grid_clear(struct halfword_grid_state *grid);
int  halfword_grid_push(struct halfword_grid_state *grid, s32 row, s32 col);
u32  halfword_grid_count(const struct halfword_grid_state *grid);
u16  halfword_grid_cell_at(const struct halfword_grid_state *grid, u32 index);
u32  halfword_grid_cell_row(const struct halfword_grid_state *grid, u32 index);
u32  halfword_grid_cell_col(const struct halfword_grid_state *grid, u32 index);

void halfword_writer_session_init(struct halfword_writer_session *session,
                                  u16 *cells,
                                  u32 capacity);
void halfword_writer_session_clear(struct halfword_writer_session *session);
void halfword_writer_session_capture_context(struct halfword_writer_session *session,
                                             ico_ptr32 entry_callsite,
                                             ico_ptr32 return_pc,
                                             u32 world_state_raw);
int  halfword_writer_session_push(struct halfword_writer_session *session,
                                  s32 row,
                                  s32 col);
void halfword_writer_session_set_final_counter(struct halfword_writer_session *session,
                                               u32 final_counter);
int  halfword_writer_session_is_single_cell(const struct halfword_writer_session *session);
int  halfword_writer_session_matches_bucket(const struct halfword_writer_session *session,
                                            u32 final_counter,
                                            u32 writes);
u32  halfword_writer_session_cell_count(const struct halfword_writer_session *session);
u32  halfword_writer_session_world_state(const struct halfword_writer_session *session);
int  halfword_writer_session_is_main_callsite(const struct halfword_writer_session *session);
int  halfword_writer_session_is_second_callsite(const struct halfword_writer_session *session);
int  halfword_writer_session_world_state_is_observed(const struct halfword_writer_session *session);
enum halfword_writer_bucket halfword_writer_session_bucket(const struct halfword_writer_session *session);

#endif /* ICO_HALFWORD_RUNTIME_H */
