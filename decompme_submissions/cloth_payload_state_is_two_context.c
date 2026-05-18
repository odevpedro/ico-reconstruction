typedef unsigned int ico_u32;
typedef int ico_ptr32;

int cloth_payload_state_is_two(ico_ptr32 *ctx) {
    ico_ptr32 entity = *(ico_ptr32 *)((char *)ctx + 0x15C);
    ico_ptr32 payload = *(ico_ptr32 *)((char *)entity + 0x800);
    return (*(ico_u32 *)((char *)payload + 0x48) ^ 2) < 1;
}
