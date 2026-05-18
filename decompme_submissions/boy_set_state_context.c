typedef unsigned int u32;
typedef int ico_ptr32;

struct entity_context;

void boy_set_state(struct entity_context *entity, u32 state)
{
    ico_ptr32 scene_obj;
    ico_ptr32 wk;

    scene_obj = *(ico_ptr32 *)((char *)entity + 0x15C);
    wk = *(ico_ptr32 *)((char *)scene_obj + 0x800);
    *(u32 *)((char *)wk + 0) = state;
}
