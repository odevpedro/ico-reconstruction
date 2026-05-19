#include "halfword_runtime.h"

static u32 append_str(char *dst, u32 dst_size, u32 pos, const char *src)
{
    if (dst == 0 || dst_size == 0 || src == 0)
        return pos;

    while (*src != 0 && pos + 1 < dst_size)
        dst[pos++] = *src++;

    if (pos < dst_size)
        dst[pos] = 0;

    return pos;
}

const char *halfword_writer_session_recommendation(const struct halfword_writer_session *session)
{
    if (session == 0)
        return "capture_unknown";

    if (halfword_writer_session_is_second_callsite(session))
        return "prioritize_second_callsite";

    if (halfword_writer_session_is_main_callsite(session) &&
        halfword_writer_session_bucket(session) == HALFWORD_BUCKET_SINGLE_NO_WRITE)
        return "probe_fast_path";

    if (!halfword_writer_session_world_state_is_observed(session))
        return "seek_new_world_state";

    if (halfword_writer_session_bucket(session) == HALFWORD_BUCKET_MULTI_WRITE)
        return "keep_same_route";

    return "continue_capture";
}

u32 halfword_writer_session_consume(char *dst, u32 dst_size, const struct halfword_writer_session *session)
{
    u32 pos = 0;

    if (dst == 0 || dst_size == 0)
        return 0;

    dst[0] = 0;

    pos = append_str(dst, dst_size, pos, "halfword_consume{recommend=");
    pos = append_str(dst, dst_size, pos, halfword_writer_session_recommendation(session));
    pos = append_str(dst, dst_size, pos, ",");
    pos = halfword_writer_session_format(dst + pos, (u32)(dst_size - pos), session) + pos;

    if (pos < dst_size)
        dst[pos] = 0;

    return pos;
}

