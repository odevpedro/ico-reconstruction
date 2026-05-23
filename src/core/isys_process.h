#ifndef ICO_ISYS_PROCESS_H
#define ICO_ISYS_PROCESS_H

#include "../types.h"

// ============================================================================
// GObj (Game Object) stride 0x174 — struct central do isysGObj* system
// Alocado por isysGObjAlloc, inicializado por isysGObjAdd.
// Array em gp-0x4C50 com gp-0x4C4C slots.
// Confirmed: Rev.098-099 (byte-exact .s em src/core/asm/)
// ============================================================================
struct isys_gobj {
    ico_ptr32 self;             // +0x00: auto-ref (0 = slot livre)
    ico_s32   prev;             // +0x04: irmao anterior (-1 = none)
    ico_s32   next;             // +0x08: proximo irmao (-1 = none)
    u32       type_flags;       // +0x0C: tipo/flags (zerado pelo add)
    ico_ptr32 list_link_fwd;    // +0x10: encadeamento lista direta (type DL)
    ico_ptr32 list_link_bwd;    // +0x14: encadeamento lista reversa
    u8        type_byte;        // +0x18: byte de tipo (andi 0xFF do arg a1)
    u8        pad_19[0x0F];     // +0x19..0x27: desconhecido (15B gap)
    ico_ptr32 direct_callback;  // +0x28: callback direto (chamado em _iosOmMain Pass 1)
    ico_ptr32 child_process;    // +0x2C: head da lista de processos filhos (TCBs)
    u32       field_30;         // +0x30: desconhecido (zerado pelo add)
    ico_ptr32 chain_next;       // +0x34: proximo GObj na cadeia global (iosOmCreateDL iteracao)
    ico_ptr32 pad_38;           // +0x38: gap
    ico_ptr32 type_list_next;   // +0x3C: proximo na lista por tipo (isysGObjRemove)
    ico_ptr32 pad_40[2];        // +0x40..0x47: gap (8B)
    ico_ptr32 dl_callback;      // +0x48: callback de display list (iosOmCreateDL dispatches)
    u32       slot_mask;        // +0x4C: bitmask de participacao em slots
    u32       type_bits;        // +0x50: bits de tipo/grupo (AND com dispatch node+0x50)
    ico_ptr32 pad_54;           // +0x54: gap
    u32       field_58;         // +0x58: desconhecido (zerado pelo add)
    u8        type_data[0x100]; // +0x5C..0x15B: dados especificos por tipo (256B)
                                // Nunca acessado diretamente por funcoes isysGObj*.
                                // Provavelmente acessado via user_data ptr (+0x28).
                                // Zona de 0x5C-0x15F confirmada vazia no .s (Rev.100).
    u32       field_15C;        // +0x15C: desconhecido (zerado alloc + add)
    u32       pad_160;          // +0x160: gap
    u32       field_164;        // +0x164: desconhecido (zerado pelo add)
    u32       pad_168;          // +0x168: gap
    u32       avail_flag;       // +0x16C: flag de disponibilidade (usado por _iosOmMain/iosOmCreateDL)
    u32       active_flag;      // +0x170: flag de ativo (checado em _iosOmMain Pass 1/2)
};

// ============================================================================
// Thread Control Block (TCB) stride 0x94 — "process node" do isysGObj* system
// Alocado por isysGObjProcAdd_ (0x13F3F0), init por process_node_init (ios/thread.c).
// Array em gp-0x4C48 com gp-0x4C44 slots.
// Confirmed: Rev.099 (ios/thread.c, byte-exact .s)
// ============================================================================
struct isys_thread {
    ico_ptr32 self;             // +0x00: auto-ref (0 = slot livre)
    ico_ptr32 parent_gobj;      // +0x04: GObj pai
    ico_ptr32 prev;             // +0x08: no anterior (lista duplamente ligada)
    ico_ptr32 next;             // +0x0C: proximo no
    u32       type_mask;        // +0x10: mascara de tipo (andi 0xFF)
    u32       priority;         // +0x14: prioridade (ordenacao)
    u32       active;           // +0x18: 1 = ativo
    ico_ptr32 callback_fn;      // +0x1C: funcao callback
    u8        stack_area[0x78]; // +0x24: area de pilha/init (0x94 - 0x1C)
};

// ============================================================================
// Dispatch node (32-slot dispatch table at 0x281AB0)
// Ponteiro para lista ligada de nos de dispatch, cada um com:
// Confirmed: iosOmCreateDL.s byte-exact
// ============================================================================
// struct dispatch_node {
//     ???       field_00..0x33;  // cabecalho (nao mapeado)
//     ico_ptr32 next;            // +0x34: proximo no na lista
//     ???       field_38..0x47;  // (gap?)
//     ico_ptr32 callback;        // +0x48: funcao callback deste slot
//     u32       type_bits;       // +0x50: bits de tipo (AND com GObj+0x50)
//     ???       field_54..0x16B; // (nao mapeado)
//     u32       avail;           // +0x16C: flag de disponibilidade
// };
//
// Slot stride = 4 (array de ponteiros em 0x281AB0, 32 slots)
// Cada slot e head de uma lista ligada de dispatch nodes.
// Populado por isysGObjProcAdd_ / isysGObjDlInit.

// ============================================================================
// isysGObj* system GP-relative variables
// Confirmados por analise estatica (Rev.098-099)
// ============================================================================
// Ptrs:
//   gp-0x4C50: gobj_array    (array de struct isys_gobj, stride 0x174)
//   gp-0x4C48: thread_array  (array de struct isys_thread, stride 0x94)
//   gp-0x671C: gobj_list_head (head da cadeia global de GObjs)
// Counters:
//   gp-0x4C4C: gobj_count    (numero de slots de GObj)
//   gp-0x4C44: thread_count  (numero de slots de thread)
//   gp-0x6740: thread_id_counter (contador de IDs de thread)
//   gp-0x4BE4: vblank_count  (contador VBlank em 0x274EC0)
// Flags:
//   gp-0x6724: mask_bitfield (bits 0-7, controla slots em _iosOmMain)
//   gp-0x6720: field_6720   (desconhecido, init por isysGObjInit)
// Work:
//   gp-0x6714: current_gobj (escrito em _iosOmMain antes de processar)
//   gp-0x6710: current_child_process (escrito no passo 2 de _iosOmMain)
// Loader:
//   gp-0x5828: load_stage    (indice do estagio atual de la_load_processing)
//   gp-0x58B0: load_param_A
//   gp-0x58B8: load_param_B
//   gp-0x58AC: load_param_C
//   gp-0x58A4: load_result_flag
// Heap:
//   gp-0x68E8: heap_ptr      (ponteiro base do heap)

// ============================================================================
// Tabelas
// ============================================================================
// 0x281A70: GObj type display list heads (8 entries x 4B = 32B), type byte indexada
// 0x281A90: GObj type display list tails (8 entries x 4B = 32B)
//           Populada por isysGObjInit, iterada por _iosOmMain Pass 1 (mask slots 0-7)
//
// 0x281AB0: Process dispatch table (32 slots x 4B = 128B)
//           Cada entrada e head de lista ligada de dispatch nodes.
//           Populada por isysGObjDlInit + isysGObjProcAdd_.
//           Iterada por iosOmCreateDL (per-GObj, 32 slots, mask gp-0x6724)
//
// 0x616FD0: Jump table de la_load_processing (21 entradas, 7 handlers unicos)
//
// 0x6A93D0: Type handler table (67 entries x 4B = 268B)
//           Mapeia type_byte -> handler function ptr.
//           Usada por isysGObjRemove para remocao por tipo.
//
// 0x6A6F30: Thread table (indexada por thread_id, stride 4)
//           Armazena ponteiros para struct isys_thread.
//           Counter de ID em gp-0x6740.

// ============================================================================
// Constants
// ============================================================================
#define GOBJ_STRIDE         0x174
#define THREAD_STRIDE       0x94

#define GOBJ_GP_ARRAY       (-0x4C50)
#define GOBJ_GP_COUNT       (-0x4C4C)
#define GOBJ_GP_LIST_HEAD   (-0x671C)

#define THREAD_GP_ARRAY     (-0x4C48)
#define THREAD_GP_COUNT     (-0x4C44)
#define THREAD_GP_ID_COUNTER (-0x6740)

#define MASK_GP_BITFIELD    (-0x6724)
#define MASK_GP_UNKNOWN     (-0x6720)

#define CURRENT_GOBJ        (-0x6714)
#define CURRENT_CHILD_PROC  (-0x6710)

#define LOAD_STAGE          (-0x5828)
#define LOAD_PARAM_A        (-0x58B0)
#define LOAD_PARAM_B        (-0x58B8)
#define LOAD_PARAM_C        (-0x58AC)
#define LOAD_RESULT_FLAG    (-0x58A4)

#define VBLANK_COUNTER      0x274EC0
#define HEAP_GP_PTR         (-0x68E8)

#define GOBI_TYPE_DL_HEAD_TABLE  0x281A70
#define GOBI_TYPE_DL_TAIL_TABLE  0x281A90
#define PROC_DISPATCH_TABLE      0x281AB0
#define TYPE_HANDLER_TABLE       0x6A93D0
#define THREAD_TABLE             0x6A6F30
#define SCENE_LOADER_JT          0x616FD0

#define GOBI_TYPE_DL_COUNT  8
#define PROC_DISPATCH_SLOTS  32
#define MASK_SLOTS           8
#define TYPE_HANDLER_COUNT   67

#define IOSOM_TYPE_MIN       0x13
#define IOSOM_TYPE_MAX       0x1B

#endif
