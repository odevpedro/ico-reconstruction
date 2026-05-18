typedef int ico_u32;
typedef unsigned int ico_ptr32;

int cloth_get_variant(ico_ptr32 *ctx) {
    ico_ptr32 entity = *(ico_ptr32 *)((char *)ctx + 0x15C);
    ico_ptr32 payload = *(ico_ptr32 *)((char *)entity + 0x800);
    return *(int *)((char *)payload + 4);
}
