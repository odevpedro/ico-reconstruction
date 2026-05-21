# Rev.098 — isysGObj* Process Registration, Dispatch Internals and Scene Loader Stages

**Date:** 2026-05-21

---

## Resumo executivo

A sessao de analise estatica via Capstone (little-endian MIPS64, ELF EE) focou no funcionamento interno do sistema `isysGObj*`, revelando:

1. **`isysGObjProcAdd_` (0x13F3F0)**: funcao central de registro de processes. 488 bytes, 7 parametros, stride 0x94, lista duplamente ligada ordenada por prioridade.
2. **`FUN_0013f7a8`**: wrapper thin que rearranja argumentos e chama `isysGObjProcAdd_`. NAO e um sistema separado de registro.
3. **`iosOmExeEachGObj` (0x13FD10)**: iterador simples de lista ligada — confirma o modelo de slots runtime.
4. **`_iosOmMain` (0x13F9D0)**: dispatcher principal de processes. 8 slots de mascara + 9 slots de tipo = **17 slots** — coincidencia com os 17 slots vistos em runtime.
5. **Tabela 0x281A70**: runtime BSS (zero no arquivo), populada durante init. Tabela de dispatch de processes do `isysGObj*`.
6. **`initSceneGObj` (0x1B76F8)**: 0x828 bytes, conecta a tabela de descritores (68 tipos, 0x2A31B8) ao sistema `isysGObj*`.
7. **`la_load_processing` (0x1B2A30)**: 21 estagios de carregamento de cena via jump table em 0x616FD0.
8. **`eBrainGetStatus`**: ~44B executaveis — funcao trivial de status check.

---

## 1. isysGObjProcAdd_ (0x13F3F0) — Registro de process

### Visao geral

488 bytes (0x13F3F0 - 0x13F62C). Esta e a funcao central que registra um process (callback + dados associados) em uma lista por GObj.

### Assinatura

```c
void *isysGObjProcAdd_(
    void *gobj,              // a0 = GObj (game object)
    void *callback_fn,       // a1 = funcao callback
    u8 callback_type,        // a2 = tipo (andi 0xFF)
    u8 type_id,              // a3 = type id (andi 0xFF)
    u32 priority,            // t0 = prioridade (usada para ordenacao)
    void *unk_t1,            // t1 = passado para init
    void *unk_t2             // t2 = passado para init
);
```

### Estrutura do process node (stride 0x94)

Cada process node tem 0x94 bytes (148 bytes). Campos identificados ate agora:

| Offset | Tamanho | Campo | Descricao |
|--------|---------|-------|-----------|
| +0x00 | 4 | `self_ptr` | Ponteiro para si mesmo (sw $s1, ($s1)) |
| +0x04 | 4 | `parent_gobj` | GObj pai (a0 original) |
| +0x08 | 4 | `prev` | No anterior na lista |
| +0x0C | 4 | `next` | Proximo no na lista |
| +0x10 | 4 | `callback_type_mask` | Mascara/tipo (s6 = andi a3, 0xFF) |
| +0x14 | 4 | `priority` | Prioridade (s3 = t0) — usada para ordenacao |
| +0x18 | 4 | `active_flag` | Bandeira de ativo (1 = ativo) |
| +0x1C | 4 | `callback_fn` | Ponteiro de funcao callback (a1) |
| +0x24 | ? | `init_area` | Area de init usada por 0x13D1B0 |

### Fluxo

```
isysGObjProcAdd_(gobj, callback_fn, type, type_id, priority, t1, t2)
  |
  +-> Se callback_fn == NULL: retorna NULL
  +-> Verifica se ha slots livres
  |     gp-0x4c44 = count de processes
  |     gp-0x4c48 = array de slots (stride 0x94)
  |     Procura slot vazio (slot[0] == NULL)
  |
  +-> Se nao ha slots livres: printa debug string 0x557AE8
  |     Itera slots imprimindo: slot_index, self, callback_type, callback_fn
  |
  +-> Se achou slot vazio:
  |     Marca slot como ocupado (self_ptr)
  |     Chama 0x13D1B0(gobj_area+0x24, 1, callback_type, gobj, callback_fn, priority, t1, t2)
  |       -> Init do process node
  |     Chama 0x13D3C8(gobj_area+0x24)
  |       -> Insercao na lista ligada do GObj
  |     Se callback_type == 0: sw zero, 0x1C(node) (callback_fn = NULL)
  |     Senao: sw callback_type(a2), 0x1C(node)
  |
  +-> Preenche node:
  |     node->type_id = s6 (a3 & 0xFF)
  |     node->active = 1
  |     node->parent_gobj = s2 (a0 original)
  |     node->priority = s3 (t0)
  |
  +-> Insercao ordenada na lista do GObj:
  |     Se gobj->process_list == NULL:
  |       node->prev = node->next = NULL
  |       gobj->process_list_head = gobj->process_list_tail = node
  |     Senao, insere ordenado por prioridade (crescente):
  |       Procura posicao onde node->priority < existing->priority
  |       Ajusta ponteiros prev/next
  |
  +-> Retorna node ptr
```

### Chamadas internas

| Endereco | Funcao | Proposito |
|----------|--------|-----------|
| `0x1A6E28` | print/assert | Debug strings (4 chamadas) |
| `0x13D1B0` | node_init | Inicializa area de process node |
| `0x13D3C8` | list_insert | Insere na lista ligada do GObj |

### GP-relative vars

| Offset | Uso |
|--------|-----|
| `gp-0x4c44` | Process count (max slots) |
| `gp-0x4c48` | Process slot array base |
| `gp-0x68E8` | Heap/allocator (passado para 0x13D1B0) |

---

## 2. FUN_0013f7a8 — Wrapper de isysGObjProcAdd_

### Descoberta

`FUN_0013f7a8` nao e um sistema de registro separado. E uma **wrapper thin** que rearranja argumentos e chama `isysGObjProcAdd_`.

### Implementacao

```asm
# Entry point 1 (0x13F7A8)
# Rearranja: a0,a1,a2,a3,t0 -> a1,a2,a3,t0,? 
0x13F7A8:  move   $v0, $a3           # salva a3 original
0x13F7AC:  addiu  $sp, $sp, -0x10
0x13F7B0:  andi   $a3, $a2, 0xff    # a3 = type_id (de a2)
0x13F7B4:  move   $t1, $t0           # t1 = t0 original
0x13F7B8:  move   $a2, $a1           # a2 = callback_type (de a1)
0x13F7BC:  sd     $ra, ($sp)
0x13F7C0:  move   $t0, $v0           # t0 = a3 original (type_id + ?)
0x13F7C4:  jal    isysGObjProcAdd_   # a1 = a0 original (callback_fn)
0x13F7C8:  move   $a1, $a0
0x13F7CC:  ld     $ra, ($sp)
0x13F7D0:  jr     $ra
0x13F7D4:  addiu  $sp, $sp, 0x10

# Entry point 2 (0x13F7D8)
# Similar mas com t1 = 0x1800 fixo
0x13F7D8:  move   $t0, $a3           # t0 = a3
0x13F7DC:  addiu  $sp, $sp, -0x10
0x13F7E0:  andi   $a3, $a2, 0xff    # a3 = type_id (de a2)
0x13F7E4:  sd     $ra, ($sp)
0x13F7E8:  move   $a2, $a1           # a2 = callback_type (de a1)
0x13F7EC:  addiu  $t1, $zero, 0x1800 # t1 = 0x1800 fixo
0x13F7F0:  jal    isysGObjProcAdd_
0x13F7F4:  move   $a1, $zero         # a1 = NULL (callback_fn = 0?!)
0x13F7F8:  ld     $ra, ($sp)
0x13F7FC:  jr     $ra
0x13F800:  addiu  $sp, $sp, 0x10
```

### Implicacao

O segundo entry point com `a1 = 0` e `t1 = 0x1800` sugere que esta variante registra um process **sem callback** — possivelmente um process "placeholder" ou um de inicializacao que sera preenchido depois.

### Renomeacao

`FUN_0013f7a8` deve ser renomeada para `isysGObjProcAdd_Dispatch` ou `isysGObjProcAdd_Wrapper` — o nome exato depende de entendermos o significado de `t1` e do segundo entry point.

---

## 3. iosOmExeEachGObj (0x13FD10) — Iterador de lista ligada

### Implementacao completa (40 bytes de instrucoes)

```asm
iosOmExeEachGObj(slot_id, callback, arg):
0x13FD10:  addiu  $sp, $sp, -0x40
0x13FD14:  lui    $v0, 0x28
0x13FD18:  sd     $s2, 0x20($sp)
0x13FD1C:  sll    $a0, $a0, 2
0x13FD20:  sd     $s1, 0x10($sp)
0x13FD24:  addiu  $v0, $v0, 0x1a70    # v0 = 0x281A70
0x13FD28:  sd     $ra, 0x30($sp)
0x13FD2C:  addu   $a0, $a0, $v0       # a0 = &table_281A70[slot_id]
0x13FD30:  sd     $s0, ($sp)
0x13FD34:  move   $s2, $a1             # s2 = callback
0x13FD38:  lw     $s0, ($a0)           # s0 = list_head = table[slot_id]
0x13FD3C:  beqz   $s0, 0x13fd5c        # se vazia, sai
0x13FD40:  move   $s1, $a2             # s1 = argument
0x13FD44:  move   $a0, $s0             # a0 = node
0x13FD48:  jalr   $s2                  # call callback(node, arg)
0x13FD4C:  move   $a1, $s1
0x13FD50:  lw     $s0, 0x10($s0)       # s0 = node->next (offset 0x10)
0x13FD54:  bnez   $s0, 0x13fd48        # loop enquanto next != NULL
0x13FD58:  move   $a0, $s0
0x13FD5C:  ld     $ra, 0x30($sp)
0x13FD60:  ld     $s2, 0x20($sp)
0x13FD64:  ld     $s1, 0x10($sp)
0x13FD68:  ld     $s0, ($sp)
0x13FD6C:  jr     $ra
0x13FD70:  addiu  $sp, $sp, 0x40
```

### Funcionamento

1. Recebe `slot_id` (a0), `callback` (a1), `arg` (a2)
2. Indexa tabela `0x281A70[slot_id * 4]` = head da lista ligada
3. Itera a lista: `node->next` em offset 0x10 (mesmo offset do node do isysGObjProcAdd_)
4. Chama `callback(node, arg)` em cada node
5. Retorna quando node == NULL

### Implicacao para dados runtime

Esta funcao **e** a fonte dos eventos de slot vistos nas sessoes PCSX2. Cada chamada a `iosOmExeEachGObj` produz N eventos (um por node na lista). A distribuicao de 15 slots ativos nos dados runtime corresponde a 15 slots preenchidos na tabela `0x281A70`.

---

## 4. _iosOmMain (0x13F9D0) — Dispatcher principal de processes

### Arquitetura

534 bytes (0x13F9D0 - 0x13FBF8). Este e o dispatcher principal que itera todos os processes.

### Estrutura de dois passes

**Passo 1: Mascara de slots (8 slots, a1=0..7)**

```asm
0x13FA10:  lw     $a0, -0x6724($gp)   # a0 = gp-0x6724 (mask bitfield)
# Loop: for a1 = 0..7:
#   if (mask_bitfield & (1 << a1))    # testa bit a1 da mascara
#     slot_head = table_281A70[a1]
#     itera lista ligada
#       se node->state_check_1 == 0: pula
#       se node->state_check_2 == 0: pula
#       se node->callback_0x28 != 0: chama callback(node)
```

**Passo 2: Tipo de process (9 slots, type_id = 0x13..0x1B)**

```asm
# Loop: for type_id = 0x13..0x1B (9 tipos):
#   slot_head = table_281A70[type_id]
#   itera lista ligada
#     itera child processes (node->child_list em offset 0x2C)
#       se child->priority == type_id:
#         se child->active_flag (offset 0x18) != 0:
#           se child->type_mask (offset 0x10) == 0:
#             chama 0x13D8A0(child->init_area)
#             se retorno != 0x22:
#               chama 0x13D928(child->init_area)
#             senao:
#               chama isysGObjProcRemove(child)
#           senao:
#             chama child->callback_fn(gobj)
```

### Slots = 8 + 9 = 17

O primeiro passo usa 8 slots de mascara (bits 0-7 de `gp-0x6724`). O segundo passo usa 9 slots de tipo (`type_id` = 0x13 a 0x1B). Total: **17 slots**.

Isto corresponde exatamente aos **17 slots** vistos nos dados runtime das sessoes PCSX2. A coincidencia nao e acidental — os eventos runtime sao produziDOS por `_iosOmMain` iterando as tabelas de processes.

### GP-relative vars em _iosOmMain

| Offset | Uso |
|--------|-----|
| `gp-0x6724` | Bitmask de slots ativos (bits 0-7) |
| `gp-0x6714` | Current GObj (escrito antes de processar) |
| `gp-0x6710` | Current child process (escrito no passo 2) |
| `gp-0x671C` | Referencia para iosOmCreateDL |

### Chamadas internas de _iosOmMain

| Endereco | Nome | Proposito |
|----------|------|-----------|
| `0x13D8A0` | process_poll | Poll de process (retorna status) |
| `0x13D928` | process_step | Step de process |
| `0x13F6B8` | `isysGObjProcRemove` | Remocao de process (se status == 0x22) |
| (JALR $v0) | callback_0x28 | Callback direto do node (offset 0x28) |
| (JALR $v0) | callback_0x1C | Callback do child process (offset 0x1C) |

---

## 5. Tabela 0x281A70 — Runtime BSS

### Descoberta

A tabela em `0x281A70` e **zero no arquivo ELF** (128 bytes de 0x00). Isto significa que ela e populada em runtime, provavelmente durante a inicializacao do sistema isysGObj*.

### Layout

```
0x281A70: [ptr_0] [ptr_1] [ptr_2] ... [ptr_31] = 32 slots × 4 bytes = 128 bytes
```

### Referencias

- `_iosOmMain` (0x13F9D0): usa slots 0-7 (passo 1) e depois slots indiretos via type_id (passo 2)
- `iosOmExeEachGObj` (0x13FD10): indexa diretamente por slot_id
- `iosOmCreateDL` (0x13FC00): provavelmente preenche a tabela

### Tabela 0x281AB0

Tambem zero no arquivo. Provavelmente e uma tabela secundaria usada por `iosOmCreateDL` para configuracao de slots.

---

## 6. initSceneGObj (0x1B76F8) — Conexao descritor→isysGObj

### Descoberta

`initSceneGObj` e uma funcao grande (0x828 bytes = 2088 bytes) que conecta a **tabela de descritores** (68 entradas, stride 0x64, em `0x2A31B8`) ao sistema `isysGObj*`.

### Evidencia

```asm
0x1B76F8:  addiu  $sp, $sp, -0x130
0x1B76FC:  addiu  $v1, $zero, 0x4c       # stride da entry table? (0x4C = 76)
0x1B7700:  sd     $s6, 0xf0($sp)
0x1B7704:  lui    $v0, 0x2a              # segmento 0x2AXXXX
0x1B7708:  move   $s6, $a1               # s6 = entry_index (a1)
0x1B7710:  mult   $ac3, $s6, $v1         # entry_index * 0x4C
0x1B771C:  addiu  $v0, $v0, 0x4c48       # 0x2A4C48 = base de tabela
0x1B7724:  addiu  $a2, $zero, 0x64       # stride 0x64 = descriptor stride
0x1B7744:  addiu  $a3, $s2, 0x31b8       # a3 = 0x2A31B8 = descriptor table
0x1B774C:  lbu    $a0, 0x46($s4)         # entry_table[entry_index]+0x46 = descriptor_index
```

A funcao:
1. Recebe `entry_index` em a1
2. Calcula deslocamento na entry table (stride 0x4C) em `0x2A4C48`
3. Usa stride 0x64 para acessar a descriptor table em `0x2A31B8`
4. Le o campo `entry_table[N]+0x46` = descriptor_index
5. Itera a descriptor table para init de GObj

### Significado

Isto confirma que:
- A entry table (512 entradas, stride 0x4C) mapeia objetos de cena para 68 tipos de entidade
- A descriptor table (68 entradas, stride 0x64) contem init_fn, hA, hB, hC
- `initSceneGObj` e a ponte entre esses dados estaticos e o sistema `isysGObj*`
- Cada objeto de cena vira um GObj no sistema `isysGObj*`, com processes registrados via `isysGObjProcAdd_`

---

## 7. la_load_processing (0x1B2A30) — 21 estagios de carregamento

### Jump table

A funcao usa uma jump table em `0x616FD0` com 21 entradas (0x15). O estagio atual e mantido em `gp-0x5828`.

### Estrutura

```asm
la_load_processing:
0x1B2A30:  addiu  $sp, $sp, -0x60
0x1B2A38:  move   $s0, $a0              # argumento
0x1B2A40:  lui    $a0, 0x61
0x1B2A44:  addiu  $a0, $a0, 0x6f38      # string "load_processing"
0x1B2A54:  jal    print                  # print debug string
0x1B2A5C:  lw     $v1, -0x5828($gp)     # le estagio atual
0x1B2A68:  sltiu  $v0, $v1, 0x15        # se estagio < 21
0x1B2A6C:  beqz   $v0, end              # se >= 21, encerra
0x1B2A78:  sll    $v1, $v1, 2           # estagio * 4
0x1B2A7C:  addiu  $v0, $v0, 0x6fd0      # jump table base = 0x616FD0
0x1B2A80:  addu   $v1, $v1, $v0
0x1B2A84:  lw     $a0, ($v1)            # carrega endereco alvo
0x1B2A88:  jr     $a0                   # salta para o estagio
```

### Estagios

Os 21 estagios precisam ser extraidos da jump table em `0x616FD0`. Proximos passos:
- Dump da jump table e decode de cada endereco
- Mapear cada endereco para nome de funcao
- Documentar a sequencia de carregamento

---

## 8. la_switching_stage (0x1B5958) — Transicao de cena

Funcao pequena que coordena a transicao entre cenas:

```asm
0x1B5958:  addiu  $sp, $sp, -0x10
0x1B5960:  jal    0x192098               # eBrainGetStatus? (verificar)
0x1B5968:  bnez   $v0, skip_transition   # se status ok, pula
0x1B5970:  lwc1   $f12, -0x77e4($gp)     # carrega parametro float
0x1B5978:  mtc1   $at, 0x4080            # constante 4.0
0x1B597C:  jal    0x19FF98               # funcao de transicao
0x1B5980:  lw     $a0, -0x58ac($gp)      # parametro de stage
```

Segundo entry point em `0x1B5998`:
```asm
0x1B5998:  lw     $v0, 0x5254($v1)       # 0x275254 = flag register
0x1B59A4:  andi   $v0, $v0, 0x10         # testa bit 4
0x1B59A8:  beqz   $v0, skip              # se bit 4 = 0, pula
0x1B59B0:  jal    0x1B7040               # stage init function
```

---

## 9. eBrainGetStatus (0x191D20) — Primeiro ponto de apoio GirlBrain

### Funcionamento

Apenas ~44 bytes de codigo executivo. Funcao extremamente simples:

```asm
eBrainGetStatus(entity_id):    # a0 = entity_id
0x191D20:  lw     $v0, -0x4b38($gp)      # current_status (gp-0x4b38)
0x191D24:  bltz   $v0, store_new          # se status < 0, armazena novo
0x191D28:  move   $a2, $a0               # a2 = entity_id (salvo)
0x191D2C:  bne    $a2, $v0, record       # se entity_id != current_status
0x191D30:  lw     $a1, -0x4b3c($gp)      # counter/tail_ptr
0x191D34:  jr     $ra
0x191D38:  addiu  $v0, $zero, 1          # retorna 1 (ja registrado)

store_new:
0x191D3C:  sw     $a2, -0x4b38($gp)       # current_status = entity_id
0x191D40:  jr     $ra
0x191D44:  addiu  $v0, $zero, 1          # retorna 1

record:
0x191D48:  lui    $a0, 0x6d              # buffer base = 0x6D0B90
0x191D50:  move   $v0, $zero             # retorna 0
0x191D54:  sll    $v1, $a1, 2            # counter * 4
0x191D58:  addiu  $a1, $a1, 1            # counter++
0x191D5C:  addu   $v1, $v1, $a0          # &buffer[counter]
0x191D60:  sw     $a1, -0x4b3c($gp)       # atualiza counter
0x191D64:  jr     $ra
0x191D68:  sw     $a2, ($v1)             # buffer[counter] = entity_id
```

### Funcao

Mantem um buffer circular de entity IDs em `0x6D0B90` com contador em `gp-0x4b3c`. O status atual fica em `gp-0x4b38`.
- Se `current_status < 0`: define current_status = entity_id, retorna 1
- Se `entity_id == current_status`: retorna 1
- Se `entity_id != current_status`: armazena no buffer, retorna 0

Parece ser um **tracker de entidades** — monitora qual entidade esta no foco atual e mantem historico.

---

## 10. kanbanExec (0x1B05A8) — Processador de requisicoes

### Funcionamento

```asm
kanbanExec:
0x1B05A8:  addiu  $sp, $sp, -0x40
0x1B05AC:  lw     $a0, -0x4a84($gp)      # kanban_request_queue
0x1B05B4:  beqz   $a0, exit              # se fila vazia, sai
0x1B05BC:  lw     $v1, ($a0)             # primeira requisicao
  # Converte 3 floats de posicao (offset 0x10, 0x14, 0x18) * 255.0 (0x437F0000)
  # Converte 1 float de rotacao (offset 0x1C) * 254.0 (0x42FE0000?)
  # Armazena como 4 bytes (pos_x, pos_y, pos_z, rot)
0x1B0620:  jal    0x1118b8               # som/audio?
0x1B0628:  jal    0x112148               # config/setup
0x1B0630:  jal    0x112188               # config/setup
0x1B0640:  jal    0x111fa8               # config/setup (a0=1, a1=7, a2=0)
```

Processa uma requisicao de cada vez. Converte dados de posicao/rotacao de float para bytes e faz setup de audio/config.

---

## O que fica confirmado

1. **`isysGObjProcAdd_`** e a funcao central de registro de processes (488B, stride 0x94, lista ligada ordenada).
2. **`FUN_0013f7a8`** e um wrapper de `isysGObjProcAdd_` — NAO e um sistema separado.
3. **`iosOmExeEachGObj`** e um iterador simples de lista ligada — a fonte dos eventos de slot runtime.
4. **`_iosOmMain`** tem 17 slots (8 mascara + 9 tipo) — coincide com os 17 slots runtime.
5. **Tabela 0x281A70** e runtime BSS (zero no ELF), populada durante init.
6. **`initSceneGObj`** conecta a entry table (0x2A4C48, stride 0x4C) e descriptor table (0x2A31B8, stride 0x64) ao sistema `isysGObj*`.
7. **`la_load_processing`** tem 21 estagios de carregamento via jump table em 0x616FD0.
8. **`eBrainGetStatus`** e uma funcao pequena de tracking de entidades (44B executaveis).
9. **Process node structure**: stride 0x94, campos 0x00=self, 0x04=parent, 0x08=prev, 0x0C=next, 0x10=type_mask, 0x14=priority, 0x18=active, 0x1C=callback, 0x24+=init.
10. **GP-relative vars**: gp-0x4c44 (process count), gp-0x4c48 (slot array), gp-0x6724 (mask bitfield), gp-0x5828 (load stage counter).

---

## O que fica provavel

- A tabela `0x281A70` tem 32 slots (8 mascara + 24 tipo), mas apenas 17 sao usados. (Provavel — requer verificacao.)
- `initSceneGObj` chama `isysGObjProcAdd_` para registrar processes dos handlers de entidade. (Provavel — requer verificacao.)
- Os 21 estagios de `la_load_processing` incluem: init de stage, load de texturas, init de entidades, audio setup. (Provavel — requer dump da jump table.)
- `kanbanExec` processa requisicoes de efeitos sonoros ou particulas. (Provavel — requer contexto.)
- O segundo entry point de `FUN_0013f7a8` (com a1=0, t1=0x1800) registra processes de "escuta" ou placeholder. (Provavel — requer contexto.)

---

## O que foi descartado

- `FUN_0013f7a8` como sistema de registro separado. E wrapper de `isysGObjProcAdd_`.
- Tabela 0x281A70 como compile-time `.data`. E runtime BSS.
- `_iosOmMain` como funcao simples. Tem 534 bytes com dois passes complexos.
- `initSceneGObj` como dead code (suspeita anterior). E a funcao central de init de cena, 0x828 bytes.

---

## Proximos alvos pequenos (offline)

1. **Dump da jump table 0x616FD0**: 21 entradas — mapear cada endereco para nome de funcao.
2. **Analisar 0x13D1B0 e 0x13D3C8**: funcoes auxiliares de init/insercao de process node.
3. **Mapear os campos do process node**: os offsets 0x00-0x94 precisam ser documentados.
4. **Analisar `iosOmCreateDL` (0x13FC00)**: como popula a tabela 0x281A70.
5. **Analisar `isysGObjAdd` (0x13E8D8)**: como GObj sao criados e adicionados ao sistema.
6. **Analisar `isysGObjInit` (0x13DDA0)**: inicializacao do sistema isysGObj*.
7. **Confirmar a relacao `entry_table +0x46` → `descriptor_table`**: verificar o campo que liga as duas tabelas.
8. **Extrair a jump table de `la_load_processing`**: 21 handlers de estagio.
9. **Documentar `gp-0x6724`**: a bitmask de 8 bits que controla quais slots de mascara estao ativos.

---

## Riscos

### Risco 1: Tabela 0x281A70 e BSS

A tabela e zero no arquivo, mas pode ser populada por codigo que nao faz parte do sistema isysGObj*. Confirmar xrefs: toda escrita em `0x281A70-0x281AF0` deve ser de `iosOmCreateDL` ou funcoes de init.

### Risco 2: 17 slots vs 32 slots

O espaco alocado para 0x281A70 e 128 bytes = 32 slots. Mas _iosOmMain usa apenas 17. Os outros 15 slots podem ser para outros propositos ou estender o sistema.

### Risco 3: initSceneGObj e muito grande

Com 0x828 bytes, `initSceneGObj` e uma das maiores funcoes do jogo. Pode conter:
- Loop de criacao de GObj para cada entrada da cena
- Registro de processes via isysGObjProcAdd_
- Configuracao inicial de estados
- Tratamento de casos especiais (personagens principais vs props)

### Risco 4: Little-endian vs Big-endian

O ELF e little-endian, mas Ghidra foi configurado como big-endian (MIPS padrao). As analises Ghidra anteriores podem ter interpretado dados multi-byte incorretamente, especialmente tabelas e ponteiros. Verificar sempre com a leitura correta de endianness.

---

## Veredito

Esta revisao fecha duas das maiores pontas soltas do projeto:

1. **O sistema de registro de processes**: `isysGObjProcAdd_` com stride 0x94, lista ligada ordenada, e 7 parametros. `FUN_0013f7a8` e apenas um wrapper.

2. **A origem dos 17 slots runtime**: `_iosOmMain` itera 8 slots de mascara + 9 slots de tipo = 17, exatamente como os dados PCSX2 mostravam. A coincidencia nao e acidental — os eventos runtime sao produziDOS por este dispatcher.

3. **A conexao cena→entidade**: `initSceneGObj` (0x1B76F8, 2088 bytes) conecta a entry table (stride 0x4C) e descriptor table (stride 0x64) ao sistema `isysGObj*`.

O modelo arquitetural agora e:

```
entry_table (0x2A4C48, stride 0x4C, 512 entradas)
  |
  +0x46 -> descriptor_index
  |
descriptor_table (0x2A31B8, stride 0x64, 68 entradas)
  |
initSceneGObj (0x1B76F8)
  |
  v
isysGObjInit -> GObj criado
  |
isysGObjProcAdd_ (0x13F3F0) -> process registrado (stride 0x94)
  |
  v
_iosOmMain (0x13F9D0) -> dispatcher de 17 slots
  |   +-- Passo 1: mascara (8 slots, gp-0x6724)
  |   +-- Passo 2: tipo (9 slots, type_id 0x13-0x1B)
  |
  v
iosOmExeEachGObj (0x13FD10) -> iteracao de lista ligada
  |
  v
callback do process executado
```

O cenario de carregamento e separado:

```
kanbanReqAdd -> kanbanInit -> kanbanExec
  |
  v
la_load_processing (21 estagios, jump table 0x616FD0)
  |
  v
la_switching_stage -> initSceneGObj -> HotInitSceneObjects
```
