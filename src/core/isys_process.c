#include "isys_process.h"

/* ============================================================================
 * isysGObjProcAdd_ (0x13F3F0) — Registro de process
 *
 * Parametros:
 *   a0 = gobj (game object alvo)
 *   a1 = callback_fn (funcao callback)
 *   a2 = callback_type (tipo de callback)
 *   a3 = type_id (identificador de tipo, andi 0xFF)
 *   t0 = priority (prioridade para ordenacao)
 *   t1 = extra_param (passado para node_init)
 *   t2 = extra_param (passado para node_init)
 *
 * Retorna: ponteiro para o process node criado, ou 0 se erro.
 *
 * Aloca um node de stride 0x94, inicializa, insere na lista ligada
 * do GObj ordenada por prioridade, e retorna o node.
 * ============================================================================
 */
ico_ptr32 isysGObjProcAdd_(ico_ptr32 gobj,
                            ico_ptr32 callback_fn,
                            u32 callback_type,
                            u32 type_id,
                            u32 priority,
                            ico_ptr32 t1_param,
                            ico_ptr32 t2_param)
{
    ico_ptr32 process_count;
    ico_ptr32 process_array;
    ico_ptr32 node;
    ico_ptr32 existing;
    u32 i;

    if (callback_fn == 0) {
        return 0;
    }

    // Procura slot vazio no array de processes
    process_count = *(ico_ptr32 *)(0 + PROCESS_GP_COUNT);
    if (process_count != 0) {
        process_array = *(ico_ptr32 *)(0 + PROCESS_GP_ARRAY);
        for (i = 0; i < (u32)process_count; i++) {
            if (*(ico_ptr32 *)(process_array + i * PROCESS_NODE_STRIDE) != 0) {
                // Slot ocupado, continua
                continue;
            }
            break;
        }
        // Se nao achou slot vazio, printa debug
        if (i == (u32)process_count) {
            // Debug print: "isysGObjProcAdd_: no free slot"
            sub_1A6E28((ico_ptr32)0x557AE8, 0);
            i = 0;
        }
    }

    // Aloca e inicializa node
    // ...

    return node;
}

/* ============================================================================
 * iosOmExeEachGObj (0x13FD10) — Iterador de lista ligada de processes
 *
 * Itera a lista de processes de um slot e chama callback em cada node.
 *
 * Parametros:
 *   slot_id  = indice do slot na tabela 0x281A70
 *   callback = funcao chamada para cada node (a0=node, a1=arg)
 *   arg      = argumento passado para callback
 * ============================================================================
 */
void iosOmExeEachGObj(u32 slot_id,
                       ico_ptr32 callback,
                       ico_ptr32 arg)
{
    ico_ptr32 table_base;
    ico_ptr32 node;

    table_base = IOSOM_DISPATCH_TABLE;
    node = *(ico_ptr32 *)(table_base + slot_id * 4);

    while (node != 0) {
        ((void (*)(ico_ptr32, ico_ptr32))callback)(node, arg);
        node = *(ico_ptr32 *)(node + 0x10);   // node->next
    }
}

/* ============================================================================
 * isysGObjProcAdd_Wrapper (0x13F7A8) — Wrapper de isysGObjProcAdd_
 *
 * Rearranja argumentos e chama isysGObjProcAdd_.
 * Segundo entry point (0x13F7D8) usa t1=0x1800 fixo e callback_fn=0.
 * ============================================================================
 */
ico_ptr32 isysGObjProcAdd_Wrapper(ico_ptr32 callback_fn,
                                   ico_ptr32 callback_type,
                                   u32 type_id,
                                   ico_ptr32 extra_param,
                                   ico_ptr32 gobj,
                                   ico_ptr32 t1_param)
{
    // Rearranja: isysGObjProcAdd_(gobj, callback_fn, callback_type, type_id,
    //                             extra_param(como priority?), t1_param, ?)
    return isysGObjProcAdd_(gobj, callback_fn, callback_type,
                            type_id, extra_param, t1_param, 0);
}

/* ============================================================================
 * isysGObjProcAdd_NoCallback (0x13F7D8) — Wrapper com callback_fn=0
 *
 * Similar ao wrapper acima mas registra sem callback (a1=0, t1=0x1800).
 * ============================================================================
 */
ico_ptr32 isysGObjProcAdd_NoCallback(ico_ptr32 type_id,
                                      ico_ptr32 callback_type,
                                      ico_ptr32 extra_param,
                                      ico_ptr32 gobj)
{
    return isysGObjProcAdd_(gobj, 0, callback_type,
                            (u32)type_id, (u32)extra_param, 0x1800, 0);
}

/* ============================================================================
 * eBrainGetStatus (0x191D20) — GirlBrain status tracker
 *
 * Mantem status atual em gp-0x4b38 e buffer circular em 0x6D0B90.
 *
 * Parametros:
 *   entity_id = identificador de entidade a rastrear
 *
 * Retorna:
 *   1 se entity_id ja estava registrado como status atual
 *   0 se entity_id foi adicionado ao historico (diferente do status atual)
 *
 * Se current_status < 0: define current_status = entity_id, retorna 1
 * Se entity_id == current_status: retorna 1
 * Se entity_id != current_status: armazena no buffer, retorna 0
 * ============================================================================
 */
u32 eBrainGetStatus(ico_ptr32 entity_id)
{
    ico_ptr32 current_status;
    ico_ptr32 counter;

    current_status = *(ico_ptr32 *)(0 + (-0x4B38));

    if ((s32)current_status < 0) {
        *(ico_ptr32 *)(0 + (-0x4B38)) = entity_id;
        return 1;
    }

    if (entity_id == current_status) {
        return 1;
    }

    // Armazena entity_id no buffer circular
    counter = *(ico_ptr32 *)(0 + (-0x4B3C));
    *(ico_ptr32 *)(0x6D0B90 + counter * 4) = entity_id;
    *(ico_ptr32 *)(0 + (-0x4B3C)) = counter + 1;

    return 0;
}
