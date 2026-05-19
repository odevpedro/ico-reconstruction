#include "halfword_runtime.h"

const char *halfword_writer_bucket_name(enum halfword_writer_bucket bucket)
{
    switch (bucket) {
    case HALFWORD_BUCKET_EMPTY:
        return "empty";
    case HALFWORD_BUCKET_SINGLE_NO_WRITE:
        return "single_no_write";
    case HALFWORD_BUCKET_SINGLE_ONE_WRITE:
        return "single_one_write";
    case HALFWORD_BUCKET_MULTI_WRITE:
        return "multi_write";
    default:
        return "unknown";
    }
}

const char *halfword_writer_session_callsite_name(const struct halfword_writer_session *session)
{
    if (session == 0)
        return "unknown";

    if (halfword_writer_session_is_main_callsite(session))
        return "main_callsite";

    if (halfword_writer_session_is_second_callsite(session))
        return "second_callsite";

    return "other_callsite";
}

const char *halfword_writer_session_world_state_name(const struct halfword_writer_session *session)
{
    if (session == 0)
        return "unknown";

    if (!halfword_writer_session_world_state_is_observed(session))
        return "unobserved_world_state";

    return "observed_world_state";
}

