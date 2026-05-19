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

static u32 append_u32_hex(char *dst, u32 dst_size, u32 pos, u32 value)
{
    static const char hex[] = "0123456789abcdef";
    char buf[10];
    int i;

    buf[0] = '0';
    buf[1] = 'x';
    for (i = 0; i < 8; i++) {
        buf[2 + i] = hex[(value >> ((7 - i) * 4)) & 0xF];
    }
    buf[10 - 1] = 0;
    return append_str(dst, dst_size, pos, buf);
}

static u32 append_u32_dec(char *dst, u32 dst_size, u32 pos, u32 value)
{
    char buf[11];
    int i = 10;

    buf[i] = 0;
    do {
        buf[--i] = (char)('0' + (value % 10u));
        value /= 10u;
    } while (value != 0u && i > 0);

    return append_str(dst, dst_size, pos, &buf[i]);
}

u32 halfword_writer_session_format(char *dst, u32 dst_size, const struct halfword_writer_session *session)
{
    u32 pos = 0;

    if (dst == 0 || dst_size == 0)
        return 0;

    dst[0] = 0;

    if (session == 0) {
        return append_str(dst, dst_size, 0, "halfword_session=<null>");
    }

    pos = append_str(dst, dst_size, pos, "halfword_session{bucket=");
    pos = append_str(dst, dst_size, pos, halfword_writer_bucket_name(halfword_writer_session_bucket(session)));
    pos = append_str(dst, dst_size, pos, ",callsite=");
    pos = append_str(dst, dst_size, pos, halfword_writer_session_callsite_name(session));
    pos = append_str(dst, dst_size, pos, ",world=");
    pos = append_str(dst, dst_size, pos, halfword_writer_session_world_state_name(session));
    pos = append_str(dst, dst_size, pos, ",entry=");
    pos = append_u32_hex(dst, dst_size, pos, (u32)session->entry_callsite);
    pos = append_str(dst, dst_size, pos, ",return=");
    pos = append_u32_hex(dst, dst_size, pos, (u32)session->return_pc);
    pos = append_str(dst, dst_size, pos, ",count=");
    pos = append_u32_dec(dst, dst_size, pos, session->count);
    pos = append_str(dst, dst_size, pos, ",writes=");
    pos = append_u32_dec(dst, dst_size, pos, session->observed_writes);
    pos = append_str(dst, dst_size, pos, ",final=");
    pos = append_u32_dec(dst, dst_size, pos, session->final_counter);
    pos = append_str(dst, dst_size, pos, ",world_state_raw=");
    pos = append_u32_hex(dst, dst_size, pos, session->world_state_raw);
    pos = append_str(dst, dst_size, pos, "}");
    return pos;
}

