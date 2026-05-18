typedef int ico_ptr32;
typedef unsigned long long ico_u64;

struct cloth_context;

int cloth_test_state_lt_2(struct cloth_context *ctx) {
    ico_ptr32 entity = *(ico_ptr32*)((char*)ctx + 0x15C);
    ico_ptr32 payload = *(ico_ptr32*)((char*)entity + 0x800);
    int result = 0;
    int extra = *(int*)((char*)ctx + 0x16C);
    if (extra != 0) {
        ico_u64 flag = *(ico_u64*)((char*)payload + 8);
        if (flag == 0) {
            __asm__ __volatile__("nop");
            result = *(int*)((char*)payload + 0x48) < 2;
        }
    }
    return result;
}
