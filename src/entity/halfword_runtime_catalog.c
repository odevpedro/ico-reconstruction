#include "halfword_runtime.h"

/*
 * Halfword runtime catalog
 *
 * This module records the current, validated observation set as small,
 * queryable data instead of burying it in notes only. It stays conservative:
 * each record names a measured address or cluster and the next probe that
 * still matters from that observation.
 */

static const struct halfword_runtime_catalog_entry halfword_runtime_catalog_entries[] = {
    {
        "main_callsite",
        "confirmed",
        "dominant caller into 0x00166BB0; Rev.095 produced 61,504,387 entries",
        "probe_fast_path",
        (ico_ptr32)0x0016700C,
        61504387u,
    },
    {
        "second_callsite",
        "runtime_confirmed",
        "rare caller seen in Rev.094; absent in the Rev.095 capture",
        "replay_second_callsite",
        (ico_ptr32)0x0016828C,
        14257u,
    },
    {
        "fast_path",
        "inferred",
        "0x00166DFC remains unobserved directly in the current capture",
        "probe_fast_path",
        (ico_ptr32)0x00166DFC,
        0u,
    },
    {
        "world_state_cluster",
        "confirmed",
        "Rev.095 clustered world_state_raw values: 0x13/0x14/0x15/0x0A/0x16/0x12/0x09/0x32/0x17/0x0B/0x08/0x01",
        "seek_new_world_state",
        (ico_ptr32)0x00000000,
        12u,
    },
};

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
    char buf[11];
    int i;

    buf[0] = '0';
    buf[1] = 'x';
    for (i = 0; i < 8; i++)
        buf[2 + i] = hex[(value >> ((7 - i) * 4)) & 0xF];
    buf[10] = 0;

    return append_str(dst, dst_size, pos, buf);
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

static int halfword_runtime_catalog_label_eq(const char *lhs, const char *rhs)
{
    if (lhs == 0 || rhs == 0)
        return 0;

    while (*lhs != 0 && *rhs != 0) {
        if (*lhs != *rhs)
            return 0;
        lhs++;
        rhs++;
    }

    return *lhs == 0 && *rhs == 0;
}

static u32 halfword_runtime_catalog_count_internal(void)
{
    return (u32)(sizeof(halfword_runtime_catalog_entries) /
                 sizeof(halfword_runtime_catalog_entries[0]));
}

u32 halfword_runtime_catalog_count(void)
{
    return halfword_runtime_catalog_count_internal();
}

const struct halfword_runtime_catalog_entry *halfword_runtime_catalog_at(u32 index)
{
    if (index >= halfword_runtime_catalog_count_internal())
        return 0;
    return &halfword_runtime_catalog_entries[index];
}

const struct halfword_runtime_catalog_entry *halfword_runtime_catalog_find(const char *label)
{
    u32 i;

    if (label == 0)
        return 0;

    for (i = 0; i < halfword_runtime_catalog_count_internal(); i++) {
        if (halfword_runtime_catalog_label_eq(halfword_runtime_catalog_entries[i].label, label))
            return &halfword_runtime_catalog_entries[i];
    }

    return 0;
}

const struct halfword_runtime_catalog_entry *halfword_runtime_catalog_latest(void)
{
    u32 count = halfword_runtime_catalog_count_internal();

    if (count == 0)
        return 0;

    return &halfword_runtime_catalog_entries[count - 1];
}

const char *halfword_runtime_catalog_default_next_probe(void)
{
    const struct halfword_runtime_catalog_entry *latest = halfword_runtime_catalog_latest();

    if (latest == 0 || latest->next_probe == 0)
        return "continue_capture";

    return latest->next_probe;
}

u32 halfword_runtime_catalog_format_entry(char *dst, u32 dst_size, const struct halfword_runtime_catalog_entry *entry)
{
    u32 pos = 0;

    if (dst == 0 || dst_size == 0)
        return 0;

    dst[0] = 0;

    if (entry == 0)
        return append_str(dst, dst_size, 0, "halfword_catalog_entry=<null>");

    pos = append_str(dst, dst_size, pos, "halfword_catalog_entry{label=");
    pos = append_str(dst, dst_size, pos, entry->label);
    pos = append_str(dst, dst_size, pos, ",status=");
    pos = append_str(dst, dst_size, pos, entry->status);
    pos = append_str(dst, dst_size, pos, ",address=");
    pos = append_u32_hex(dst, dst_size, pos, (u32)entry->address);
    pos = append_str(dst, dst_size, pos, ",hits=");
    pos = append_u32_dec(dst, dst_size, pos, entry->hits);
    pos = append_str(dst, dst_size, pos, ",next=");
    pos = append_str(dst, dst_size, pos, entry->next_probe);
    pos = append_str(dst, dst_size, pos, ",detail=");
    pos = append_str(dst, dst_size, pos, entry->detail);
    pos = append_str(dst, dst_size, pos, "}");
    return pos;
}

u32 halfword_runtime_catalog_format_all(char *dst, u32 dst_size)
{
    u32 count;
    u32 i;
    u32 pos = 0;
    char line[512];

    if (dst == 0 || dst_size == 0)
        return 0;

    dst[0] = 0;
    count = halfword_runtime_catalog_count_internal();

    for (i = 0; i < count; i++) {
        u32 written = halfword_runtime_catalog_format_entry(line, sizeof(line), &halfword_runtime_catalog_entries[i]);
        if (written >= sizeof(line))
            written = (u32)(sizeof(line) - 1);
        line[written] = 0;
        pos = append_str(dst, dst_size, pos, line);
        if (i + 1 < count)
            pos = append_str(dst, dst_size, pos, "\n");
    }

    return pos;
}
