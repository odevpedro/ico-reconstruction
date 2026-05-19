#include "halfword_runtime.h"

/*
 * Halfword runtime classification helpers
 *
 * This is a local, conservative bridge from the Rev.094/Rev.096 capture data
 * into code. It does not claim gameplay meaning beyond the observed buckets and
 * the callsite / world-state clustering seen in runtime logs.
 */

#define HALFWORD_MAIN_CALLSITE   0x0016700Cu
#define HALFWORD_SECOND_CALLSITE 0x0016828Cu

static int halfword_runtime_world_state_is_common(u32 world_state_raw)
{
    switch (world_state_raw) {
    case 0x01u:
    case 0x08u:
    case 0x09u:
    case 0x0Au:
    case 0x0Bu:
    case 0x0Cu:
    case 0x0Du:
    case 0x0Eu:
    case 0x0Fu:
    case 0x10u:
    case 0x11u:
    case 0x12u:
    case 0x13u:
    case 0x14u:
    case 0x15u:
    case 0x16u:
    case 0x17u:
    case 0x32u:
        return 1;
    default:
        return 0;
    }
}

int halfword_writer_session_is_main_callsite(const struct halfword_writer_session *session)
{
    if (session == 0)
        return 0;
    return session->entry_callsite == (ico_ptr32)HALFWORD_MAIN_CALLSITE;
}

int halfword_writer_session_is_second_callsite(const struct halfword_writer_session *session)
{
    if (session == 0)
        return 0;
    return session->entry_callsite == (ico_ptr32)HALFWORD_SECOND_CALLSITE;
}

int halfword_writer_session_world_state_is_observed(const struct halfword_writer_session *session)
{
    if (session == 0)
        return 0;
    return halfword_runtime_world_state_is_common(session->world_state_raw);
}

enum halfword_writer_bucket halfword_writer_session_bucket(const struct halfword_writer_session *session)
{
    if (session == 0)
        return HALFWORD_BUCKET_UNKNOWN;

    if (session->final_counter == 0 && session->observed_writes == 0)
        return HALFWORD_BUCKET_EMPTY;

    if (session->final_counter == 1 && session->observed_writes == 0)
        return HALFWORD_BUCKET_SINGLE_NO_WRITE;

    if (session->final_counter == 1 && session->observed_writes == 1)
        return HALFWORD_BUCKET_SINGLE_ONE_WRITE;

    if (session->observed_writes > 1 || session->final_counter > 1)
        return HALFWORD_BUCKET_MULTI_WRITE;

    return HALFWORD_BUCKET_UNKNOWN;
}

