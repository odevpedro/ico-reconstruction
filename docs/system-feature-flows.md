# System & Feature Flows — ICO Reconstruction

> Documento vivo. Atualizado sempre que uma feature for criada ou modificada.
> **Ultima atualizacao:** 2026-05-17 (Rev.072 — Room init callback field corrigido: +0x154 de row_base (= +0x174 absoluto), 19 function pointers pre-carregados; instrucao 0x1AF954 corrigida para mult (nao and); NOTA: 0x001AF190 NAO e scene loader — e init de sala que le da tabela 404)

---

## Indice

- [Cloth Physics Dispatch Flow](#cloth-physics-dispatch-flow)
- [Physics Object Type Table Initialization](#physics-object-type-table-initialization)
- [Callback Registration Flow](#callback-registration-flow)
- [Scene Loader Flow](#scene-loader-flow)

---

## Entry Table Descriptor Iteration Flow

> **Status:** Fluxo mapeado (Rev.050). Funcao `0x1B76F8` identificada como iteradora.

### Visao Geral

O sistema de inicializacao cloth do ICO usa uma tabela de entries (entrada/sala/zona) em `0x002A4C48` (stride 0x4C) que referencia descritores em `0x002A31B8` (stride 0x64). Cada descritor tem 3 slots de callback que controlam o ciclo de vida de objetos fisicos.

O fluxo completo e:

```
Entry Table (0x002A4C48, stride 0x4C)
  |
  | entry[+0x46] = descriptor index
  v
Descriptor Table (0x002A31B8, stride 0x64)
  |
  | descriptor+0x58 = cloth_payload_init (handler_c)
  | descriptor+0x50 = callback update (handler_b)
  | descriptor+0x48 = callback auxiliar (handler_a)
  v
Physics Type Table (0x001A48A0, stride 0x64) -- nomes APENAS, handlers duplicados
```

### Fluxo detalhado

```
1. Load room/zone data
   └─> Entry table (0x002A4C48) populada com entries por zona

2. 0x1B76F8(entry_index) -- Descriptor iteration
   ├─> Calcula entry VA = 0x002A4C48 + entry_index * 0x4C
   ├─> Le descriptor_idx = entry[+0x46]
   ├─> Le entry[+0x2C], entry[+0x30] (parametros numericos)
   ├─> Testa condicoes de tipo (idx == 0x0F?, 0x04?, 0x21?)
   │
   ├─> Setup espacial
   │   ├─> Le floats de entry[+0x0C..+0x20] (posicao/rotacao/escala)
   │   ├─> Calculos com mul.s, div.s, neg.s (transformacao)
   │   └─> Chama descriptor[+0x38] se nao-nulo (callback opcional)
   │
   ├─> Prepara descritor alvo
   │   ├─> s0 = 0x002A31B8 + descriptor_idx * 0x64
   │   ├─> Le entry[+0x48] >> 14 & 7 (subcategoria)
   │   ├─> Le entry[+0x47] & 0x1f (subtype)
   │   ├─> Chama 0x240D40(s0, ...)
   │   ├─> Chama 0x19F310() -- alocacao
   │   ├─> [context+0x15C] = retorno (cria entidade)
   │   └─> [entity+0x814] = entry[+0x30]
   │
   ├─> INIT: Chama cloth_payload_init
   │   ├─> lw v0, [descriptor + 0x58]  -- callback init
   │   ├─> beqz v0, skip
   │   ├─> jalr v0
   │   │   ├─> a0 = context
   │   │   └─> a1 = sp (initializer na stack!)
   │   └─> [[context+0x15C] + 0x800] = retorno (payload)
   │
   └─> REGISTER: Registra callback update (opcional)
       ├─> Se entry[+0x24] != NULL:
       │   └─> 0x13F7A8(context, entry[+0x24], 0, a3=0x13)
       ├─> Senao, se descriptor[+0x40] != NULL:
       │   └─> 0x13F7A8(context, descriptor[+0x40], 0, a3=0x13)
       └─> Se descriptor_idx == 1 (BOY): salva context em global
           Se descriptor_idx == 2 (GIRL): salva context em outro global

3. Per-frame update
   └─> Node system (0x13F9D0/0x13FB70) ou chamada direta
       └─> Chama descriptor[+0x50] (handler_b = 0x001D3A30)
           └─> ROPE callback prepara contexto
                └─> Chama dispatcher 0x001D37C8
                    └─> state_id de [payload + 0x48] → jump table 0x00618FB0
                        ├─> State 0 (0x1D3818): GUARD — check 0x1F2148. If pass: setup_1, setup_2, [payload+0x64]=1, state=1
                        ├─> State 1 (0x1D3844): PREPARE/TIMING — seed quaternion from 0x4C4750, timer calc, counter--. If 0: state=2
                        ├─> State 2 (0x1D391C): SIMULATE — matrix init, CD collision (0x12ABE0/0x12AC28), callbacks, result alloc. state=3
                        ├─> State 3 (0x1D39E0): CHECK — 0x12A7F8. If pass: state=4, [payload+0]=1, clear extra. Else: retry
                        └─> State 4 (0x1D3A10): IDLE/DONE — no-op. External code resets state to 0
```

### Funcoes envolvidas

| Funcao | Endereco | Status | Tamanho | Proposito |
|--------|----------|--------|---------|-----------|
| Entry iteration | `0x001B76F8` | Mapeado | ~1024B+ | Itera entry table, chama inits |
| Cloth init (handler_c) | `0x001D27A8` | Decompilado parcial | 528B | Inicializa payload cloth |
| Cloth core init | `0x001B7FE8` | Nao tentado | ??? | Init chain (chamado de 0x1D27A8) |
| Cloth physics setup | `0x001F2240` | Nao tentado | ??? | Physics setup (chamado de 0x1D27A8) |
| Transform setup | `0x0019F4E8` | Nao tentado | ??? | Camera/transform (chamado de 0x1D27A8) |
| Matrix init | `0x00105F00` | Nao tentado | ??? | Matrix init (chamado de 0x1D27A8) |
| Sound/event reg | `0x001AE6F8` | Nao tentado | ??? | Chamado em ambos paths de 0x1D27A8 |
| ROPE callback (handler_b) | `0x001D3A30` | Confirmado runtime | ??? | Update callback |
| Dispatcher | `0x001D37C8` | NEAR-STRUCTURAL | 616B | 5-state FSM (guard→prepare→simulate→check→done) |
| Node callback dispatcher | `0x0013FB70` | Mapeado | ??? | Node system dispatch |
| Callback register | `0x0013F7A8` | Mapeado | ??? | Registra callback no node system |

### Por que 0x001D3A30 nunca aparece nos logs de 0x13F7A8

O callback update `0x001D3A30` esta em `descriptor[+0x50]` do BARREL. O registro via `0x13F7A8` em `0x1B76F8` usa `entry[+0x24]` (override) ou `descriptor[+0x40]`:

- Entries BARREL tem `entry[+0x24] = 0` (sem override)
- BARREL descriptor tem `descriptor[+0x40] = 0x00000000` (sem callback padrao para registro)

Portanto, `0x13F7A8` NUNCA e chamado para entries BARREL. O callback `0x001D3A30` e chamado DIRETAMENTE, provavelmente pelo node system ou por outro dispatcher que le `descriptor[+0x50]`.

### Por que BARREL e nao ROPE

A entry table (dados de sala) usa **BARREL** (indice 0x13) para objetos cloth. O nome **ROPE** (indice 0x14) existe no descriptor table e no physics type table, mas NUNCA e referenciado pela entry table. BARREL e ROPE compartilham os mesmos 3 handlers, entao sao equivalentes em termos de comportamento runtime.

### O que falta

- Identificar o dispatcher que chama descriptor[+0x50] em runtime (provavelmente node system)
- Decompilar o dispatcher (resolver `.word` vs `.dword` da jump table)
- Decompilar os 5 state blocks
- Entender como a entry table e populada para diferentes zonas

---

## Live Dispatch Flow — Slot Table + Runtime Pointer List

> **Status:** Modelo consolidado (Rev.066-067). Slot table em 0x00282690 (17 entries), callbacks parametricos (2 templates), runtime ptr list em 0x006AAC80.

### Visao Geral

O sistema de live dispatch do ICO usa dois mecanismos independentes:

1. **Slot table** (`0x00282690`, 17 entries stride 0x10 — confirmado via `sll $a1,$a1,4` em 0x00166E1C): cada entry tem 4 campos de 4 bytes: w0 (grupo), w1 (guarda), w2 (flag extra), callback ptr. **14 callbacks unicos** em 17 slots (slots 8/9/16 reusam callbacks de slots 1/3/12).
2. **Runtime pointer list** (`0x006AAC80`, populada por `0x00166028`): ponteiros para entidades/contextos a processar.

O dispatcher `0x00166E10` combina os dois: para cada slot index, ele carrega a config da slot table e itera a runtime pointer list, aplicando filtros e chamando callbacks.

### Fluxo completo

```
Inicializacao:
  0x00166028() --- constroi lista runtime em 0x006AAC80 (max 0x100 entries, stride 4)
    └─ gp-25896 = count
    └─ 0x006AAC80[0..count-1] = pointers

  gp-25856 e gp-25852 sao inicializados com:
    └─ a0 == 0: 0x00167230 / 0x00167258 (cold paths, imagem inicial ELF)
    └─ a0 != 0: 0x00169F80 / 0x0016A058 (implementacao alternativa com extra init)
    └─ Configurado por 0x00168650 (tail-call J em 0x001A3334, ou via wrappers slots 12-16)

Dispatch (por slot index a1 = 0..16):
  Wrapper (0x001683A8-0x00168628)
    └─ a0 = context pointer
    └─ a1 = slot index (0..16)
    └─ a1 0-11: gp-25856 (cold path A ou alt A)
    └─ a1 12-16: gp-25852 (cold path B ou alt B)
    │
    ├─ Cold path A (0x00167230): limpa +0xB0/+0x94/+0x88, copia default state gp-25904 para +0x80
    │  └─ tail-call J 0x00166E10
    │
    ├─ Cold path B (0x00167258): limpa +0x94, copia default state gp-25904 para +0x8C
    │  └─ tail-call J 0x00166E10
    │
    ├─ Alt A (0x00169F80): JAL cold path A, depois extra init (80B frame, 6 callees, 2x 0x1D4A58)
    │  └─ jr $ra
    │
    └─ Alt B (0x0016A058): JAL cold path B, depois extra init (constantes deslocadas -0x20)
       └─ jr $ra
    │
    └─── dispatcher 0x00166E10:
          │
          1. slot_entry = 0x00282690 + a1 * 0x10
          2. callback = slot_entry[+0x0C]
          3. w0 = slot_entry[+0x00]  (Group 1 or 2)
          4. w1 = slot_entry[+0x04]  (duplicate guard)
          5. w2 = slot_entry[+0x08]  (extra flag)
          6. matrix_init via 0x243B60 x3
          7. Loop sobre runtime_ptr_list em 0x006AAC80:
             ├─ Para cada entry na lista (count gp-25896):
             │  ├─ Filtra por [ctx+0x74/0x78/0x7C] (se w1==1)
             │  └─ Se passar:
             │     └─ Dispatch callback via 0x00167020:
             │        └─ v1 = callback target
             │        └─ a0 = ctx (s1)
             │        └─ a1 = entry (s2)
             │        └─ a2 = loop index (s0)
             │        └─ JALR v1
             │           └─ Se v0 == 0: executa fallback copy path em 0x00167048
             │
             └─ Fim do loop

Callback (vindo de slot_table[N].callback):
  14 funcoes parametricas, todas com mesmo skeleton:
    addiu $sp, -0x90, salva $ra+$s0-$s7, itera halfword table em 0x6AB080 (BSS)

    ├─ Group 1 (w0=1, slots 0-11, elem_size=0x50):
    │  ├─ Chama JAL 0x166258(a0=ctx, a1=struct*, a2=idx, a3=mode)
    │  │  └─ Setup de posicao/rotacao (matriz, float ops)
    │  └─ Escreve [ctx+0x88]=struct*, [ctx+0x80]=a1, [ctx+0x84]=a2
    │
    └─ Group 2 (w0=0, slots 12-16, elem_size=0x70):
       ├─ Chama JAL 0x1667E0(a0=ctx, a1=struct*, a2=idx)
       │  └─ Check de orientacao (dot products, bc1t)
       └─ Escreve [ctx+0x94]=struct*, [ctx+0x8C]=a1, [ctx+0x90]=a2, [ctx+0x88]=0
```

### Funcoes envolvidas

| Funcao | VA | Tamanho | Proposito |
|--------|-----|---------|-----------|
| Dispatcher main body | 0x00166E10 | ~400B stack, 250+ insns | Itera runtime list, dispatch por slot |
| Cold path A | 0x00167230 | — | Leaf fragment: limpa +0xB0/+0x94/+0x88, copia default, J para main |
| Cold path B | 0x00167258 | — | Leaf fragment: limpa +0x94, copia default offset +0x8C, J para main |
| Alternate A | 0x00169F80 | 80B frame | JAL cold path A + extra init (6 callees, 2x 0x1D4A58) |
| Alternate B | 0x0016A058 | 80B frame | JAL cold path B + extra init (constantes deslocadas) |
| Runtime list builder | 0x00166028 | — | Constroi lista em 0x006AAC80, max 0x100 |
| Slot initializer | 0x00168650 | — | Configura gp-25856/gp-25852 (cold vs alt) |
| Group 1 sub | 0x166258 | 0xE0 stack | Setup de posicao/rotacao |
| Group 2 sub | 0x1667E0 | 0x30 stack | Check de orientacao |
| Matrix lib | 0x243B60 | — | Matrix copy/transform (PS2 SDK) |

### Internals dos templates Group1 e Group2

**Group 1 template (0x00166258) — Position/Rotation Proximity Check:**
- Stack: -224 bytes, salva s0-s4, ra, f20-f24
- Entrada: a0=context, a1=entity, a2=iteration_flag
- Fluxo:
  1. Carrega posicao alvo de [entity+76] (data pointer)
  2. Memcpy da posicao de referencia de [context+32] para stack
  3. Calcula deltas: ΔX = ref - entity.X, ΔZ = ref - entity.Z
  4. Comparacoes FPU com thresholds configuraveis (por eixo)
  5. Operacoes LQ/PS (VU0 SIMD) para comparacao em quadword
  6. Retorna 0 (sem match) ou 1 (match)

**Group 2 template (0x001667E0) — Orientation/Origin Matching:**
- Stack: -48 bytes
- Entrada: a0=context, a1=entity
- Fluxo:
  1. Carrega dados de orientacao 4-component de [entity+64..80]
  2. Carrega posicao do contexto de [context+32..40]
  3. Calcula diferencas de componentes via FPU sub.s
  4. Normaliza via div.s por 1.0f
  5. Itera lista ligada (stride 16 bytes) comparando contra threshold

**Callback skeleton (representative: 0x00168DA8):**
Todos os 14 callbacks compartilham este padrao:
  1. Le contador halfword de GP-19396 (0x633D2C)
  2. Itera tabela em 0x6AB080
  3. Extrai `row = halfword >> 5`
  4. Lookup row em tabela secundaria de ponteiros
  5. Itera lista ligada (structs de 80 bytes cada)
  6. Chama template Group1 ou Group2 para cada candidato
  7. Se match: armazena ponteiro do objeto em context, retorna 1
  8. Se nao: retorna 0

### Tabela de slots

| Slot | w0 | w1 | Callback | Grupo | Guarda | Mascara +0x48 |
|-----:|---:|---:|---------|-------|--------|---------------|
| 0 | 1 | 0 | 0x168DA8 | Group1 | — | (nenhuma) |
| 1 | 1 | 0 | 0x168ED0 | Group1 | — | 0xF000 / 0xF==1 |
| 2 | 1 | 0 | 0x1692F0 | Group1, a2=1 | — | 0xF000 / 0xF==1 |
| 3 | 1 | 0 | 0x169440 | Group1 | — | 0xF000 |
| 4 | 1 | 1 | 0x169020 | Group1 | Triplet | 0xF000 / 0xF==1 |
| 5 | 1 | 1 | 0x169190 | Group1 | Triplet | 0xF000 |
| 6 | 1 | 0 | 0x1696C0 | Group1 | — | 0xC000==0x4000 |
| 7 | 1 | 0 | 0x169580 | Group1 | — | 0x3000!=0 |
| 8 | 1 | 0 | 0x168ED0 | Group1 (w2=1) | — | (reusa slot 1) |
| 9 | 1 | 0 | 0x169440 | Group1 (w2=1) | — | (reusa slot 3) |
| 10 | 1 | 0 | 0x169800 | Group1 | — | 0x7000 / 0xC000==0x8000 |
| 11 | 1 | 0 | 0x169968 | Group1 | — | 0xC000==0xC000 |
| 12 | 0 | 0 | 0x169AA8 | Group2 | — | (nenhuma) |
| 13 | 0 | 1 | 0x169BD0 | Group2 | Triplet | — |
| 14 | 0 | 0 | 0x169E58 | Group2, a2=1 | — | — |
| 15 | 0 | 0 | 0x169D18 | Group2 | — | [+0x60]&0xF==2 |
| 16 | 0 | 0 | 0x169AA8 | Group2 (w2=1) | — | (reusa slot 12) |

### Halfword table em 0x006AB080 (BSS)

Todos os 14 callbacks iteram esta tabela runtime-populada. Ela contem uint16 entries que mapeiam IDs de objeto para indices em uma tabela de ponteiros. O loop percorre a tabela, e para cada indice valido, carrega o struct correspondente de `table_base[index]` e verifica o campo `+0x48` contra a mascara do slot.

### O que falta

- Validacao runtime: capturar quais slots disparam durante gameplay vs cutscene vs menu
- Capturar se a implementacao alternativa (0x00169F80/0x0016A058) e atingida
- Entender o significado semantico de cada slot (ex: slot 0 = BOY? slot 2 = GIRL?)

---

## VU0 Ring Buffer Packet Flow

> **Status:** Mapeado (Rev.069). Packet builder 0x1D43F8, push function 0x111918, VU0 kick stub 0x117C40.

### Visao Geral

O sistema de transform VU0 usa um buffer circular em `0x004C7710` para enviar pacotes VIF ao VU0 microcode. O packet builder `0x1D43F8` é chamado de `0x1D4A58` (transform orchestrator), que é chamado pela implementação alternativa do dispatch (`0x169F80`/`0x16A058`).

### Fluxo

```
Alternate A/B (0x169F80/0x16A058)
  └─> JAL cold path + extra init
       └─> 0x1D4A58 (transform orchestrator, 192B frame)
            ├─> 0x1D45B0 (matrix builder — COP2 plane clip)
            ├─> Swap A↔C buffers (condicional)
            └─> 0x1D43F8 (packet builder, 96B frame) × 2 (two-pass: pass0 t0=0, pass1 t0=-1)
                 └─> 5× 0x111918 (ring_buffer_push, 16 bytes cada)
                      └─> ring_buf.head avança 16 por push
                           └─> Quando `t0=-1`: type-5 entries recebem 0xFFFFFFFF no upper 32 bits
                                └─> VIF uploader em 0x3800C interpreta como terminator de batch

Consumo:
  J 0x3800C (runtime resident, fora do ELF)
     └─> VIF packet upload → DMA → VU0 microcode
```

### Formato do pacote VU0 (80 bytes por invocacao de 0x1D43F8)

```
Entry 1: [type=0  ] [*(gp-0x54C4)]  — header
Entry 2: [type=1  ] [4 bytes a1]    — atributos de vertice 1
Entry 3: [type=5  ] [3 campos a0]   — transform matrix A (terminator se t0=-1)
Entry 4: [type=1  ] [4 bytes a3]    — atributos de vertice 2
Entry 5: [type=5  ] [3 campos a2]   — transform matrix C (terminator se t0=-1)
```

### VU0 kick stub

O stub em `0x117C40` carrega comandos VIF em 4 GPRs e tail-call `0x3800C`:

```asm
0x117C40: lui $v1, 0xE74B   ; VF3
0x117C44: lui $k1, 0xE64B   ; VF27
0x117C48: lui $s3, 0xE54B   ; VF19
0x117C4C: lui $t3, 0xE44B   ; VF11
0x117C50: andi $zero, $s0, 0x0F4A  ; timing NOP
0x117C54: j 0x03800C
```

O SQC2 block emparelhado (`0x117C80`) salva VF3/11/19/27.

### Funcoes envolvidas

| Funcao | VA | Tamanho | Proposito |
|--------|-----|---------|-----------|
| Packet builder | 0x1D43F8 | 0x1B4 (436B) | Constroi 5 entradas VIF no ring buffer |
| Ring buffer push | 0x111918 | 0x24 (36B) | Escreve sd(a1)+sd(a0), avanca head 16 |
| Transform orchestrator | 0x1D4A58 | — | Matrix init + packet submission |
| VU0 kick stub | 0x117C40 | 0x1C (28B) | Inline asm, 4 LUI + J 0x3800C |

### O que falta

- Confirmar runtime se `0x117C40` (ou `0x3800C`) e atingido durante gameplay
- Validar que o buffer circular em 0x4C7710 nao tem overflow (head avanca 80B por batch)
- Verificar se o VU0 microcode em 0x3800C corresponde ao microcode DVP ou a codigo runtime independente

---

## Room Entity Table Flow (0x005F2F98, 404-byte stride)

> **Status:** Fluxo corrigido (Rev.072). Callback field e +0x154 de row_base (= +0x174 absoluto), contem function pointers reais pre-carregados (19 non-null), NAO indice 0x4B. A funcao chamada e 0x1AF4A0 (scene init), nao 0x1AF190 (preamble separado).

### Visao Geral

O sistema de init de sala usa uma tabela de 32 entradas (stride 404) em `.data` nao-BSS, indexada pelo world_state armazenado em `gp-28512` (0x00631990). Cada entrada tem 404 bytes com nome da sala (+0x20, 32 bytes), callback pointer (row_base + 0x154), e varios campos de configuracao.

### Correcao Rev.072 — Offset do callback

O codigo em 0x1AF960 le de:

```
0x005F2FB8 + (v1) + 340
= (0x005F2F98 + 0x20) + (world_state * 404) + 340
= 0x005F2F98 + (world_state * 404) + 0x174
```

O valor 0x4B encontrado anteriormente estava em **+0x134 de row_base** (= 0x005F2F98 + 0x154), que e um campo separado (provavel debug/type ID). O campo real de callback esta em **+0x154 de row_base** (= 0x005F2F98 + 0x174) e contem **19 function pointers pre-carregados no ELF** para salas de gameplay.

### Instrucao 0x1AF954 corrigida

```
0x001AF954: mult $v1,$a0    ; v1 * 404 → hi:lo  (DEAD CODE, mflo ausente)
```

O `mult` e dead code — nenhum `mflo` segue. O `addu $v0,$v0,$v1` usa o $v1 original, que ja esta pre-multiplicado (world_state * 404) antes de chegar nesta funcao. O GP variable `0x00631990` ja contem o valor pre-multiplicado.

### Fluxo de acesso

```
Main loop (0x00101C80):
  │
  ├─ Jal 0x00166028 (build runtime pointer list)  @ 0x00101EE8
  ├─ Jal 0x001AF4A0 (scene/room init)            @ 0x00101F00
  │    │
  │    └─ 0x001AF4A0 (scene init, NAO 0x1AF190 que e preamble separado):
  │         ├─ ... outros setup ...
  │         └─ 0x001AF948-0x001AF970:
  │              ├─ v1 = [0x00631990]           ; world_state * 404 (pre-multiplicado)
  │              ├─ a0 = 404                   ; constant
  │              ├─ mult v1, a0                ; DEAD CODE (sem mflo)
  │              ├─ v0 = 0x005F2FB8            ; row base
  │              ├─ v0 = v0 + v1               ; row_base + world_state * 404
  │              ├─ v0 = *(v0 + 340)           ; load callback ptr
  │              ├─ if v0 == 0: skip           ; 13 rows tem NULL (sem gameplay)
  │              └─ jalr v0                    ; dispatch room init callback
  │
  └─ Jal 0x00166028 (novamente) via 0x1AF974 (tail do room init)
```

### Room init callbacks (19 non-null)

| Row | Room         | Callback ptr |
|-----|--------------|-------------|
|  4  | jail         | 0x00231AC8 |
|  5  | warehouse    | 0x00234AB0 |
|  7  | proto        | 0x0020FA98 |
|  8  | troko        | 0x0022A838 |
|  9  | chandelier   | 0x00228198 |
| 10  | entrance     | 0x00239750 |
| 14  | shadows      | 0x00210D78 |
| 15  | windmill     | 0x0022B878 |
| 16  | plaza        | 0x00213B88 |
| 17  | stone        | 0x0022BFE8 |
| 19  | crest_L1     | 0x0021A4D0 |
| 20  | crest_L2     | 0x0021A6B8 |
| 21  | crest_L3     | 0x0021A980 |
| 22  | taki         | 0x00211780 |
| 23  | sluice       | 0x00225F68 |
| 25  | gondola      | 0x00237B78 |
| 26  | watertower   | 0x0022D8F8 |
| 28  | crest_R1     | 0x0021F828 |
| 29  | crest_R2     | 0x0021FA30 |
| 30  | crest_R3     | 0x0021FD20 |

Null: logo (0), title(1), sacrifice(3), ico_brigde(6), gate(11), gate2(12), grave(13), symmetry_L(18), underground(24), symmetry_R(27), cliff(31). Salas sem init dedicado.

### Tabela de salas (nomes)

Ver [`research/elf/ghidra-rev071-404-table-room-names-callbacks-and-dispatch-system-consolidation.md`](../research/elf/ghidra-rev071-404-table-room-names-callbacks-and-dispatch-system-consolidation.md) para lista completa dos 32 nomes.

---

## Main Loop Flow (0x00101C80)

> **Status:** Estrutura mapeada (Rev.071). Dispatch chain completa, idle loop via VSync.

### Visao Geral

O main loop em 0x00101C80 e o ponto de entrada do frame processing. Ele executa uma cadeia de 5+ funcoes de init/dispatch por frame e, no final, decide se volta ao processamento normal ou entra em idle (VSync wait).

### Fluxo

```
0x00101C80: addiu $sp, $sp, -128
  │
  ├── Init & timer:
  │    ├─ Check gp_var (20160) → select string path
  │    ├─ jal 0x1A6E28 (print/init com string A)
  │    ├─ jal 0x1A6E28 (print/init com string B)
  │    ├─ mult/mflo timer calculation
  │    └─ jal 0x1A6E28 (print timer value)
  │
  ├── Dispatch chain (0x001EE0-0x001F08):
  │    ├─ jal 0x001AA098
  │    ├─ jal 0x00166028      ← BUILD runtime pointer list
  │    ├─ jal 0x00103370
  │    ├─ jal 0x00104C80
  │    └─ jal 0x001AF190      ← ROOM INIT (lê tabela 404-byte!)
  │         └─ [tail] jal 0x00166028 (build pointer list again)
  │
  ├── Conditional path (gp-28384 check):
  │    ├─ lw $v0, -28384($gp)
  │    ├─ if v0 != 0: skip two calls
  │    ├─ else: jal 0x00104A78, jal 0x00104AA8
  │    └─ jal 0x00103BF8
  │
  ├── Post-processing:
  │    ├─ lw + bne check (gp-28384 de novo)
  │    ├─ (conditional) jal 0x00104A78
  │    ├─ jal 0x00103FC0
  │    └─ jal 0x00101068
  │
  ├── Loop decision (0x001F50-0x001F54):
  │    ├─ lw $v0, -28384($gp)
  │    └─ if v0 == 0: beq to 0x1E10 (volta ao processamento de frame)
  │
  └── Idle/VSync (0x001F60-0x001F6C):
       └─ tight loop:
            jal 0x00104D3C    (VSync/idle wait)
            beq $zero, $zero, -3  (loop infinito ate interrupcao)
```

---

## Halfword Table Population (0x006AB080)

> **Status:** Mapeado (Rev.071). Grid rasterizacao 32x32 dentro do dispatcher.

### Visao Geral

A tabela de halfwords em `0x006AB080` (BSS, uint16) e populada por uma funcao iteradora `0x00166C80` imediatamente anterior ao dispatcher `0x00166E10`. A funcao traca uma linha/ray atraves de um grid 32x32 e registra todas as celulas intersectadas.

**Valor codificado:** `(a2 << 5) + t0` onde:
- `a2` ∈ [0,31] = coordenada Y/row
- `t0` ∈ [0,31] = coordenada X/col
- Cada halfword = `(row << 5) | col`

### Writers (exatamente 2)

| Endereco | Contexto |
|:--------:|----------|
| `0x00166D1C` | Path A: primeiros bounds-check e write |
| `0x00166D78` | Path B: segundos bounds-check e write (duplicata no mesmo loop) |

### Fluxo

```
Funcao iteradora 0x00166C80:
  └─> Loop de rasterizacao:
       ├─> a2 = coordenada Y (bounds check: 0 <= a2 < 32)
       ├─> t0 = coordenada X (bounds check: 0 <= t0 < 32)
       ├─> t7 = step X, t5 = step Y
       ├─> Se dentro dos bounds:
       │    ├─> lw a1, -0x4BC4($gp)  ; a1 = index counter (gp-19396 = 0x00633D2C)
       │    ├─> halfword = (a2 << 5) | t0
       │    ├─> v0 = counter * 2 (halfword offset)
       │    ├─> [0x6AB080 + v0] = halfword  ; store via sh
       │    └─> counter++
       ├─> Avanca a2 por t5, t0 por t7
       └─> Loop while a3 >= t2

Tabela resultante: uint16[count] em 0x6AB080
Contador: 0x00633D2C (GP-19396), resetado para 0 por sw $zero em 0x00166BDC
```

### Consumo

Todos os 14 callbacks da slot table leem o contador (GP-19396 = 30 acc) e iteram a tabela para extrair `row = halfword >> 5`, usar como indice em uma tabela secundaria de ponteiros para structs de 80 bytes, e entao testar cada struct contra o template Group1/Group2.

---

## Physics Object Type Table — Relacao com Descriptor/Entry Systems

> **Status:** Estrutura mapeada (Rev.049). Tabela e de consulta, nao de inicializacao direta.

### Visao Geral

A physics type table em `0x001A48A0` (.text, stride 0x64, 31 entries) fornece **nomes** e **handlers** para tipos de objeto fisico. Ela e uma tabela de dados, nao um registro de inicializacao.

O sistema de inicializacao real e:

```
Entry Table (0x002A4C48, .data) → Descriptor Table (0x002A31B8, .data)
```

A physics type table contem os mesmos handlers que o descriptor table, mas com indices diferentes e nomes ASCII. A relacao entre os 3 sistemas:

| Sistema | Localizacao | Stride | Funcao |
|---------|-------------|--------|--------|
| Physics Type Table | `0x001A48A0` (.text) | 0x64 | Nomes + handlers por tipo fisico (31 tipos) |
| Descriptor Table | `0x002A31B8` (.data) | 0x64 | Callbacks por label (indice 0x13 = BARREL = cloth) |
| Entry Table | `0x002A4C48` (.data) | 0x4C | Objetos por sala/zona (mecanismo de inicializacao) |

### Formato da Entry

```
+0x00: count       (1)
+0x04: handler_a   (cleanup/post-dispatch, = descriptor +0x48)
+0x08: NULL
+0x0C: handler_b   (update/callback principal, = descriptor +0x50)
+0x10: NULL
+0x14: handler_c   (init/payload, = descriptor +0x58)
+0x18: NULL
+0x1C: NULL
+0x20: name[8]    (ASCII, ex: "ROPE\0\0\0\0")
```

### Mapeamento Physics Type ↔ Descriptor

| Nome | Type Index | Descriptor Label | Descriptor Index | Handlers |
|------|-----------|-----------------|-----------------|----------|
| WOODBOX01 | 0 | NONE? | ??? | ??? |
| ROTOBJEC | 1 | ??? | ??? | 0x1C05D0/0x1C0538/0x1C00C0 |
| BARREL | 2 | NONE | 0x1A | 0x1EA030/0x1E9950/0x1E9F08 |
| ROPE | 3 (tabela .text) | BARREL | 0x13 | 0x1D3B28/0x1D3A30/0x1D27A8 |
| ROPE | 3 (tabela .text) | ROPE | 0x14 | 0x1D3B28/0x1D3A30/0x1D27A8 |

**Importante:** O nome "ROPE" aparece no physics type table como type index 2. Mas a entry table (dados de sala) nunca usa type index 2 ou descriptor ROPE. Ela usa descriptor BARREL (0x13). Os handlers sao identicos.

### Observacoes

- ROPE e ROPEFIX sao duas variantes: ROPE tem funcoes cloth (0x1Dxxxx), ROPEFIX tem funcoes overlay (0x1Exxxx)
- A tabela esta no .text, acessada provavelmente via PC-relative (loader do GCC)

---

## Entity Handler Lifecycle Flow

> **Status:** Mapeado (Rev.053-054). Padrao hC/hB/hA confirmado em 7 entidades.

### Visao Geral

Cada tipo de entidade na descriptor table tem 4 slots de funcao que controlam o ciclo de vida:

```
init_fn (+0x40) → Carregamento de assets (modelos 3D, DMA setup)
hC     (+0x58) → Constructor (alocacao de heap, init de estado)
hB     (+0x50) → Update per-frame (AI, colisao, animacao, desenho)
hA     (+0x48) → Post-init / Reset (cleanup, reset condicional)
```

### Fluxo de Inicializacao

```
1. Sistema de assets detecta nova entidade
   └─> init_fn (se != 0)
       ├─> Carrega modelos 3D via 0x1C8478 ou 0x203B78
       ├─> Configura DMA (0x202208, 0x202148)
       ├─> Configura sprites (0x203AA0)
       └─> [entity+0x15C+0x07C] = 1 (flag active)

2. Entry table iteration (0x1B76F8)
   └─> Para cada entry em zona ativa:
       ├─> Le descriptor_idx de entry[+0x46]
       ├─> Prepara initializer struct na stack (a1 = sp)
       ├─> [initializer+0x30] = entry[+0x30] (variant field)
       └─> Se descriptor[+0x58] != 0:
           └─> jalr descriptor[+0x58] (hC = CONSTRUCTOR)
               ├─> a0 = entity context
               ├─> a1 = sp (initializer struct com variant)
               ├─> 0x13A0F8(alloca) — aloca heap
               ├─> [entity_state + 0x800] = alloc_ptr
               ├─> entity_state_reg(a1, a2, a3, t0, t1)
               ├─> descriptor_setup(entity, 2)
               └─> Retorna alloc_ptr em v0

3. Per-frame update (mecanismo de dispatch runtime)
   └─> Para cada entidade ativa:
       └─> Se descriptor[+0x50] (hB) != 0:
           └─> jalr descriptor[+0x50] (hB = UPDATE)
               ├─> entity_dispatch_update(entity)
               ├─> setup_param(entity, 35, 44, tag)
               ├─> collision_check(entity, ...)
               ├─> Animacao, desenho, state machine
               └─> (0x1D3A30 = BARREL hB nunca disparou em runtime)

4. Reset/Post-init (condicional)
   └─> Se descriptor[+0x48] (hA) != 0:
       └─> jalr descriptor[+0x48] (hA = RESET)
           ├─> state_resolver()
           ├─> state_update()
           └─> Cleanup de recursos

5. Despawn
   └─> Sistema de cleanup (desconhecido)
       ├─> model_free() para cada modelo
       └─> group_free() / cleanup chain
```

### Tabela de Handlers por Entidade (Rev.056)

| Indice | Nome | init_fn | hC (constructor) | hB (update) | hA (reset) | Status |
|--------|------|---------|-------------------|-------------|------------|--------|
| 1 | BOY | 0x153478 | 0x1C1A98 | 0x1C1DD8 (51 insns) | 0x1C1F58 (80 insns) | ASM |
| 2 | GIRL | 0x174BA0 | 0x1D1668 (92 insns, 64B) | 0x1D17F8 (112 insns, anim blend) | 0x1D1A98 (16 insns) | ASM |
| 4 | ENEMY1 | 0x164440 | 0x1CE220 (~103 insns, 80B) | 0x1CE3C0 (~135 insns, AI+draw) | 0x1CE690 (~23 insns) | ASM |
| 17 | WOODBOX0 | 0x17D1D0 | 0x1C00C0 (286 insns, 400B) | 0x1C0538 (27 insns, counter) | 0x1C05D0 (28 insns) | ASM |
| 19 | BARREL | 0 | 0x1D27A8 (cloth_payload_init) | 0x1D3A30 (cloth update) | 0x1D3B28 | CLOTH |
| 30 | BGA | 0x203EE8 | 0 | 0 | 0 | ASM |
| 32 | BIRD | 0x1971C0 | 0x197240 (60 insns, 64B, random timer) | 0x197078 (2 insns, tail to 0x1E3FC8) | 0x197080 (24 insns) | NEAR |
| 46 | QUEEN | 0x19B7F8 | 0x19A7E8 (68 insns, 24B) | 0x19A8F0 (44 insns, LOD scaling) | 0x19A9A0 (32 insns) | NEAR |
| 48 | DEVIL_GI | 0x174BA0 | 0x1D1668 (= GIRL) | 0x1D17F8 (= GIRL) | 0x1D1A98 (= GIRL) | ALIAS |
| 61 | AP1 | 0x1BB6B0 | 0x1B8720 (400+ insns, 640B) | 0x1BA330 (~200 insns, 7-state FSM) | 0x1BA530 (40 insns) | ASM |
| 62 | ATTACKCH | 0x1BBF78 | 0x1BBE50 (20 insns, 12B alloc) | 0x1BBEA0 (12 insns, guard) | 0x1BBEC8 (44 insns, spawner) | NEAR |
| 63 | ATTACKCH | 0x1BBF78 | 0x1BBB20 (176 insns, dyn alloc, rotation math) | 0x1BBDD8 (28 insns, linked-list detach) | 0x1BBE48 (2 insns, NO-OP) | ASM |
| 64 | BOSS_CTR | 0x198140 | 0x198218 (60 insns, 53-slot loop) | 0x197FC8 (4 insns, conditional tail) | 0x198000 (68 insns, slot cleanup) | ASM |

### hB dispatcher diversity

Nao ha um dispatcher unificado para hB (+0x50). Cada entidade tem codigo hB independente,
chamado atraves do mesmo mecanismo de dispatcher do descritor. Padroes observados:

| Pattern | Count | Entities |
|---------|-------|----------|
| Full code | 7 | BOY, GIRL/DEVIL_GI, ENEMY1, WOODBOX0, QUEEN, AP1 |
| Delegate (tail to 0x1E3FC8) | 1 | BIRD |
| Conditional tail | 1 | BOSS_CTR (0x17DD60 or 0x17DCD8) |
| Linked-list management | 1 | ATTACKCH idx 63 |
| Small guard (12 insns) | 1 | ATTACKCH idx 62 |
| Cloth physics | 1 | BARREL |

A busca por um dispatcher centralizado (Rev.055) falhou porque o dispatcher nao existe —
o runtime le `descriptor[+0x50]` e chama `jalr` diretamente para cada entidade.

### Cloth Force Application (0x1D3F78)

```
clothSubForceApply(ctx, force_h, force_v):
  1. alloc_entity(19) → s1        // 64B heap alloc
  2. if null: return 1
  3. Loop: for each entity in chain:
     ├─ rand() * 65536.0f → s16 angle
     ├─ Check extra/count: mark for update if inactive
     ├─ [es+0x130] = force_h * sin(angle)   // X-force
     ├─ [es+0x134] = force_v * rand()        // Y-force
     ├─ [es+0x138] = force_h * cos(angle)    // Z-force
     └─ s1 = alloc_next(s1)                  // next chain entity
  4. return 1
```

**EE-only (zero COP2):** usa sin/cos do FPU do EE via 0x008BAC0/0x008BB20

### Observacoes

- **hC sempre aloca via 0x13A0F8**: todas as entidades com hC usam heap_alloc com tag unica (exceto BOSS_CTR que tem alloc=0)
- **entity_state_reg**: todas chamam com (a1, a2, a3, t0, t1) — parametros especificos por tipo
- **descriptor_setup(entity, 2)**: padrao universal (exceto BARREL que tem init_fn=0)
- **BGA e o caso atipico**: 2D sprite overlay, init_fn=12 insns, sem handlers
- **AP1 e o mais complexo**: 640B de estado, 4 child slots, state machine de 7 estados, frame offset randomization
- **DEVIL_GI = GIRL alias**: todos os 4 handlers identicos. Provavelmente paleta alternativa ou versao de cutscene.
- **Nenhum handler usa COP2**: COP2 exclusivo do sistema cloth BARREL (79 COP2 total em 2 funcoes)

---

## Callback Registration Flow

> **Status:** Fluxo completo documentado (Rev.059). Funcoes 0x13F3F0/0x13F7A8/0x13F7D8.

### Visao Geral

O sistema de registro de callbacks do ICO usa uma lista ligada de nodes (stride 0x94) armazenada em cada entidade/objeto. Callbacks sao registrados com tipo 0x13 (cloth) e armazenam um ponteiro para dados de configuracao, nao para a funcao callback em si.

### Diagrama de fluxo

```
Scene Loader (0x1B7D00)
  │
  ├─> 0x1B76F8(a0=buffer, a1=type_id)
  │     │
  │     ├─> obj = alloc_obj()          -- aloca estrutura do objeto
  │     ├─> descriptor[+0x38](stack, obj)  -- init handler
  │     ├─> if entry[+0x24] != NULL:
  │     │     0x13F7A8(obj, entry[+0x24], 0, 0x13)
  │     │   elif descriptor[+0x40] != NULL:
  │     │     0x13F7A8(obj, descriptor[+0x40], 0, 0x13)
  │     ├─> entry[+0x58](obj, stack)   -- entry handler
  │     └─> descriptor[+0x34](obj, s5) -- construct handler
  │
  └─> 0x13F7A8(a0=obj, a1=data, a2=0, a3=0x13)
        │
        └─> 0x13F3F0(a0=obj, a1=0x13, a2=data, a3=&spill)
              │
              ├─> Busca linear (max 3 nodes)
              │   Node match: armazena em [obj+0x1C]
              │
              └─> Aloca novo node via 0x1A6E28 (x2: node + body)
                    └─> Se falha: armazena em [obj+0x1C] diretamente
```

### Funcoes

| Funcao | VA | Tamanho | Descricao |
|--------|-----|---------|-----------|
| node_callback_storage | 0x13F3F0 | 576B (0x240) | Linked-list manager. Stack 144B. 2 alloc calls. |
| callback_registration | 0x13F7A8 | 44B (0x2C) | Wrapper, chama 0x13F3F0 duas vezes (main + sister) |
| callback_system_reg | 0x13F7D8 | 36B (0x24) | Variante sem contexto: a0=0x194, a1=0, t1=0x1800 |

### Registros confirmados no runtime

- 1249 chamadas de callback registration (Rev.051)
- Todas com a3=0x13 (tipo cloth)
- 10 data pointers distintos
- Nenhum e 0x1D3A30 (callback e armazenado como DATA, nao como function pointer)

### Observacoes

- **0x13F7A8** faz 2 chamadas a 0x13F3F0: primeira para obj, segunda para obj+0x10 (sister)
- **0x13F7D8** e usado para registro system-level (sem entidade, buffer fixo de 0x1800)
- **+0x1C** e o offset de armazenamento do callback, que mapeia para ThreadParam.entry no EE SDK

---

## Scene Loader Flow

> **Status:** Funcao aproximadamente em 0x1B7D00–0x1B7F00 documentada (Rev.059).

### Visao Geral

O scene loader e o ponto de entrada para inicializacao de todos os objetos de uma sala/zona. Ele itera sobre as tabelas de entries e descritores em 4 fases distintas, chamando 0x1B76F8 para criar cada objeto.

### Fases de inicializacao

```
Fase 1: Tipos base (0-5)
  Loop s0=0..5
    0x1B7D90: jal 0x1B76F8(a0=ctx, a1=s0)
  Cria: NULL(0), BOY(1), GIRL(2), ENEMY1(4), etc.

Fase 2: Re-init tipos 2-5
  Loop s0=2..5
    0x1B7DB0: jal 0x1B7B88(a0=s0)
  Segundo passo de inicializacao

Fase 3: Objetos dinâmicos
  Loop s0=s3..s1 (range de array)
    0x1B7DF8: jal 0x1B76F8(a0=ctx, a1=s0)
  Enemy objects carregados dinamicamente

Fase 4: Scan tabela 0x4B3D10 (181 entries, stride 0x40)
  Loop por 181 IDs (s1=181)
    0x1B7E6C: jal 0x1B76F8(a0=ctx, a1=entry[+2])
  Todos os IDs de objeto mapeados
```

### A funcao 0x1B76F8

Processa UM objeto/entidade por chamada:

```
0x1B76F8(a0=buffer_ctx, a1=type_id):

  01. entry = entry_table[type_id] (0x002A4C48 + type_id * 0x4C)
  02. type_idx = entry[+0x46]            -- descriptor selector
  03. desc = descriptor_table[type_idx]  (0x002A31B8 + type_idx * 0x64)
  04. s5 = alloc_obj()                  -- heap alloc do objeto
  05. if s5 == NULL: goto END
  
  06. entity_type = s5[+4] (lhu)
  07. if entity_type != type_id: goto END

  08. // Init handler from descriptor
  09. if desc[+0x38] != NULL:
  10.     desc[+0x38](stack_frame, s5)
  
  11. // Callback registration (type 19 = cloth)
  12. if entry[+0x24] != NULL:
  13.     0x13F7A8(obj, entry[+0x24], 0, 0x13)
  14. elif desc[+0x40] != NULL:
  15.     0x13F7A8(obj, desc[+0x40], 0, 0x13)

  16. // Entry table handler
  17. if entry[+0x58] != NULL:
  18.     entry[+0x58](obj, stack_frame)

  19. // Construct handler from descriptor
  20. if desc[+0x34] != NULL:
  21.     desc[+0x34](obj, s5)

  22. Store obj in global pointer (gp-based)
END:
  return
```

### Correção importante

A "physics type table" anteriormente documentada em 0x001A48A0 (Rev.049) NAO EXISTE como tabela de dados. O endereco 0x1A48A0 esta dentro de .text e decodifica como instrucao `move a0, s0`. As tabelas reais sao:

| Tabela | VA | Secao | Stride | Entries | Uso |
|--------|-----|-------|--------|---------|-----|
| Entry table | 0x002A4C48 | .data | 0x4C | 512 | Objetos por sala/zona |
| Descriptor table | 0x002A31B8 | .data | 0x64 | 68 | Descritores de tipo |

### Observacoes

- s2 e passado como contexto/buffer para toda a funcao loader (provavelmente um descritor de arquivo ou ponteiro de cena)
- A funcao 0x1B7B88 e uma segunda fase de init chamada para tipos 2-5 (NÃO 0x1B76F8 novamente)
- A tabela 0x4B3D10 tem 181 entries, stride 0x40, com type ID em [+2] (lhu)
- O offset +0x1C em cada entidade e o slot onde callbacks sao armazenados
- GIRL (type 5) nao tem handlers cloth diretamente — sua AI cria objetos cloth separadamente via BARREL descriptor
