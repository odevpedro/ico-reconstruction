typedef int ico_u32;
typedef int ico_ptr32;

struct cloth_context;

int cloth_test_variant_field(struct cloth_context *ctx) {
    ico_ptr32 entity = *(ico_ptr32*)((char*)ctx + 0x15C);
    ico_ptr32 payload = *(ico_ptr32*)((char*)entity + 0x800);
    int variant = *(int*)((char*)payload + 4);
    if ((variant ^ 1) == 0) {
        __asm__ __volatile__("nop");
        return *(int*)((char*)payload + 0x40);
    }
    return 0;
}
