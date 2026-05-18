typedef int ico_u32;
typedef int ico_ptr32;

struct cloth_context;

int cloth_test_field0_or_extra(struct cloth_context *ctx) {
    ico_ptr32 entity = *(ico_ptr32*)((char*)ctx + 0x15C);
    ico_ptr32 payload = *(ico_ptr32*)((char*)entity + 0x800);
    int result = 0;

    if (*(int*)((char*)payload + 0) == 1) {
        result = 1;
    } else if (*(ico_ptr32*)((char*)ctx + 0x16C) == 0) {
        __asm__ __volatile__("nop");
        result = 1;
    }
    return result;
}
