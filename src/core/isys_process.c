#include "isys_process.h"

/*
 * ============================================================================
 * DISPATCH ARCHITECTURE — isysGObj* system
 *
 * Dois mecanismos de dispatch independentes operam em paralelo:
 *
 * 1. _iosOmMain (0x13F9D0, 512B) — dispatch principal, 3 passes:
 *
 *    Passo 1 (mask slots 0-7):
 *      - Para cada bit em gp-0x6724 (8 bits), carrega lista de GObjs de
 *        0x281A70[slot] (GObj type display list heads)
 *      - Para cada GObj na lista, verifica active_flag (+0x170) e
 *        avail_flag (+0x16C), chama direct_callback (+0x28)
 *      - Usado para dispatch de GObj direto por tipo/mask
 *
 *    Passo 2 (type slots 0x13-0x1B, 9 tipos):
 *      - Para cada bit em gp-0x6724 (8 bits), carrega GObj de
 *        0x281A70[slot], depois itera child_process list (+0x2C)
 *      - Para cada processo filho, verifica tipo (+0x14), active (+0x18)
 *      - Se type_mask (+0x10) == 0: init path (0x13D8A0 / 0x13D928)
 *      - Se type_mask != 0: chama callback_fn (+0x1C)
 *      - Usado para thread/process dispatch hierarquico
 *
 * 2. iosOmCreateDL (0x13FC00, 264B) — dispatch por display list:
 *
 *      - Itera cadeia global de GObjs (gp-0x671C, encadeada via +0x34)
 *      - Para cada GObj, itera 32 slots (mask gp-0x6724)
 *      - Para cada slot ativo, carrega dispatch node de 0x281AB0[slot]
 *      - Dispatch node: type_bits (+0x50) AND com GObj type_bits (+0x50)
 *      - Se match: chama callback (+0x48) com node como argumento
 *      - Usado para dispatch de callback registrado por processo
 *        (isysGObjProcAdd_)
 *
 * Tabelas de dispatch:
 *   0x281A70 (8 x 4B): GObj type display list heads
 *   0x281A90 (8 x 4B): GObj type display list tails
 *   0x281AB0 (32 x 4B): Process dispatch node list heads
 *
 * Struct GObj (stride 0x174): isys_process.h struct isys_gobj
 * Struct TCB  (stride 0x94):  isys_process.h struct isys_thread
 * Dispatch node (desconhecido stride): contem fields +0x34(prev),
 *   +0x48(callback), +0x50(type_bits), +0x16C(avail)
 * ============================================================================
 */

/*
 * ============================================================================
 * isysGObjDlInit (0x13F2C8, 56B, 14 insns) — Init das tabelas DL
 *
 * Zera 8 entradas de 0x281AB0 (DL head) e 0x281AD0 (DL tail).
 * 8 slots = 8 GObj types (mask bits 0-7 do gp-0x6724).
 *
 * Nota: so 8 slots sao inicializados aqui. Os slots 8-31 da tabela
 * 0x281AB0 sao populados por isysGObjProcAdd_ em runtime.
 * ============================================================================
 */

/*
 * ============================================================================
 * isysGObjLinkObjDL (0x13F130, 156B, 39 insns) — Vincula callback ao GObj
 *
 * Parametros:
 *   a0 (gobj):       GObj alvo
 *   a1 (callback):   funcao callback (+0x48)
 *   a2 (type_byte):  byte de tipo (AND 0xFF)
 *   a3 (param4):
 *   a4 (param5):     type_bits (+0x50)
 *
 * Fluxo:
 *   1. gobj->direct_callback (+0x48) = callback
 *   2. gobj->type_bits (+0x50) = param5
 *   3. call isysGObjKindTableAdd(gobj, type_byte, param4)
 * ============================================================================
 */

/*
 * ============================================================================
 * isysGObjKindTableAdd (0x13E648, 192B, 48 insns) — Gerencia tipo no GObj
 *
 * Gerencia a tabela de tipos 0x6A93D0 (67 entries, stride 4, BSS).
 * Cada entrada e head de lista ligada de GObjs (link via +0x3C).
 *
 * Se gobj->type_flags (!= tipo_atual): remove do tipo antigo + reinsere.
 * Se tipo < 0x44 (68): insere no fim da lista em 0x6A93D0[tipo].
 *
 * Funcoes relacionadas:
 *   0x13EB50 = isysGObjKindTableGetHead(type) -> head GObj da lista
 *   0x13EBE0 = isysGObjKindTableGetNext(gobj) -> prox GObj (+0x3C)
 *   0x13E728 = isysGObjKindTableRemove(gobj)   -> remove da lista
 * ============================================================================
 */

/*
 * ============================================================================
 * isysGObjProcAdd_ (0x13F3F0, 512B) — Registro de processo/thread
 *
 * Aloca um TCB de stride 0x94 em gp-0x4C48, inicializa, insere na
 * lista ligada do GObj ordenada por prioridade.
 * ============================================================================
 */

/*
 * ============================================================================
 * GObj struct gap (0x5C-0x15B, 256B)
 *
 * Confirmado: nenhuma funcao isysGObj* acessa offsets 0x5C-0x15B
 * como campo de struct (apenas como stack frame, com basereg=$29).
 * Zona e zerada por isysGObjAdd e provavelmente acessada via
 * ponteiro indireto (user_data em +0x28) para dados especificos
 * de tipo (transform, bounding box, etc.).
 * ============================================================================
 */

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
