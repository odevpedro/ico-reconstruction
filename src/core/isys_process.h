#ifndef ICO_ISYS_PROCESS_H
#define ICO_ISYS_PROCESS_H

#include "../types.h"

// ============================================================================
// Process node (stride 0x94) — registered via isysGObjProcAdd_
// ============================================================================
struct isys_process_node {
    ico_ptr32 self;             // +0x00: ponteiro para si mesmo
    ico_ptr32 parent_gobj;      // +0x04: GObj pai
    ico_ptr32 prev;             // +0x08: no anterior (lista ligada)
    ico_ptr32 next;             // +0x0C: proximo no (lista ligada)
    u32       type_mask;        // +0x10: tipo/mascara (andi 0xFF)
    u32       priority;         // +0x14: prioridade (usada para ordenacao)
    u32       active;           // +0x18: 1 = ativo
    ico_ptr32 callback_fn;      // +0x1C: ponteiro de funcao callback
    u8        init_area[0x78];  // +0x24: area de init (0x94 - 0x1C)
};

// ============================================================================
// isysGObj* system GP-relative variables
// Confirmados por analise estatica (Rev.098)
// ============================================================================
// gp-0x4c44: process_count (numero de slots de process)
// gp-0x4c48: process_array (array de process nodes, stride 0x94)
// gp-0x6724: mask_bitfield (bits 0-7, controla slots no _iosOmMain)
// gp-0x6714: current_gobj (escrito antes de processar)
// gp-0x6710: current_child_process (escrito no passo 2)
// gp-0x5828: load_stage (estagio atual de la_load_processing)

// ============================================================================
// Tabelas
// ============================================================================
// 0x281A70: tabela de dispatch de processes (32 slots x 4B = 128B)
//           Indexada por slot_id em iosOmExeEachGObj e _iosOmMain
//           Runtime BSS — populada durante init
// 0x281AB0/0x281AD0: tabela secundaria de DL (head/tail por slot)
//           Inicializada por isysGObjDlInit e usada por isysGObjLinkObjDL
// 0x616FD0: jump table de la_load_processing (21 entradas)

// ============================================================================
// Constants
// ============================================================================
#define PROCESS_NODE_STRIDE  0x94
#define PROCESS_GP_COUNT     (-0x4C44)
#define PROCESS_GP_ARRAY     (-0x4C48)
#define PROCESS_GP_MASK      (-0x6724)

#define IOSOM_DISPATCH_TABLE   0x281A70
#define IOSOM_DL_HEAD_TABLE    0x281AB0
#define IOSOM_DL_TAIL_TABLE    0x281AD0
#define IOSOM_DISPATCH_SLOTS   32
#define IOSOM_MASK_SLOTS       8
#define IOSOM_TYPE_MIN         0x13
#define IOSOM_TYPE_MAX         0x1B

#endif
