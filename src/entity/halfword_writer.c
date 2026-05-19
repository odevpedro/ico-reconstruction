#include "halfword_runtime.h"

/*
 * Halfword writer session model
 *
 * This module tracks the state that the recent runtime captures exposed around
 * the hot writer path:
 *   - monotonic cell count
 *   - observed A/B writes
 *   - final counter bucket
 *   - entry callsite / return PC
 *   - world_state_raw
 *
 * It does not claim to be the full original function body. The goal is to keep
 * the confirmed runtime contract available in source form so later work can
 * refine it without re-deriving the bookkeeping from scratch.
 */

void halfword_writer_session_init(struct halfword_writer_session *session,
                                  u16 *cells,
                                  u32 capacity)
{
    if (session == 0)
        return;

    session->cells = cells;
    session->capacity = capacity;
    session->count = 0;
    session->observed_writes = 0;
    session->final_counter = 0;
    session->world_state_raw = 0;
    session->entry_callsite = 0;
    session->return_pc = 0;
}

void halfword_writer_session_clear(struct halfword_writer_session *session)
{
    u32 i;

    if (session == 0 || session->cells == 0)
        return;

    for (i = 0; i < session->capacity; i++)
        session->cells[i] = 0;

    session->count = 0;
    session->observed_writes = 0;
    session->final_counter = 0;
}

void halfword_writer_session_capture_context(struct halfword_writer_session *session,
                                             ico_ptr32 entry_callsite,
                                             ico_ptr32 return_pc,
                                             u32 world_state_raw)
{
    if (session == 0)
        return;

    session->entry_callsite = entry_callsite;
    session->return_pc = return_pc;
    session->world_state_raw = world_state_raw;
}

int halfword_writer_session_push(struct halfword_writer_session *session,
                                 s32 row,
                                 s32 col)
{
    if (session == 0 || session->cells == 0)
        return 0;
    if (!halfword_writer_in_bounds(row, col))
        return 0;
    if (session->count >= session->capacity)
        return 0;

    session->cells[session->count] = halfword_writer_encode((u32)row, (u32)col);
    session->count++;
    session->observed_writes++;
    return 1;
}

void halfword_writer_session_set_final_counter(struct halfword_writer_session *session,
                                               u32 final_counter)
{
    if (session == 0)
        return;

    session->final_counter = final_counter;
}

int halfword_writer_session_is_single_cell(const struct halfword_writer_session *session)
{
    if (session == 0)
        return 0;

    return session->final_counter == 1 && session->observed_writes == 0;
}

int halfword_writer_session_matches_bucket(const struct halfword_writer_session *session,
                                           u32 final_counter,
                                           u32 writes)
{
    if (session == 0)
        return 0;

    return session->final_counter == final_counter &&
           session->observed_writes == writes;
}

u32 halfword_writer_session_cell_count(const struct halfword_writer_session *session)
{
    if (session == 0)
        return 0;
    return session->count;
}

u32 halfword_writer_session_world_state(const struct halfword_writer_session *session)
{
    if (session == 0)
        return 0;
    return session->world_state_raw;
}
