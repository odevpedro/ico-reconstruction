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

static u32 append_u32_dec(char *dst, u32 dst_size, u32 pos, u32 value)
{
    char buf[11];
    int i;

    buf[10] = 0;
    i = 10;
    do {
        buf[--i] = (char)('0' + (value % 10u));
        value /= 10u;
    } while (value != 0u && i > 0);

    return append_str(dst, dst_size, pos, &buf[i]);
}

u32 halfword_runtime_catalog_snapshot(char *dst, u32 dst_size)
{
    const struct halfword_runtime_catalog_entry *latest;
    u32 pos = 0;
    u32 count;

    if (dst == 0 || dst_size == 0)
        return 0;

    dst[0] = 0;

    count = halfword_runtime_catalog_count();
    latest = halfword_runtime_catalog_latest();

    pos = append_str(dst, dst_size, pos, "halfword_catalog_snapshot{count=");
    pos = append_u32_dec(dst, dst_size, pos, count);
    pos = append_str(dst, dst_size, pos, ",next=");
    pos = append_str(dst, dst_size, pos, halfword_runtime_catalog_default_next_probe());
    pos = append_str(dst, dst_size, pos, ",latest=");
    pos = append_str(dst, dst_size, pos, latest == 0 ? "none" : latest->label);
    pos = append_str(dst, dst_size, pos, ",latest_status=");
    pos = append_str(dst, dst_size, pos, latest == 0 ? "none" : latest->status);
    pos = append_str(dst, dst_size, pos, "}");

    return pos;
}

