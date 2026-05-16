# Rev.055 — COP2 cloth decomp, BOY hC, hB dispatcher investigation

## Date

2026-05-16

## Objective

1. Decompile the two real COP2/VU0 macro-mode cloth functions
2. Analyze BOY hC constructor (0x1C1A98, ultimo handler pendente da wave 1)
3. Investigate o dispatcher runtime de descriptor[+0x50] (quem chama hB)

## Sources Used

| Fonte | Uso |
|---|---|
| `SCUS_971.13.elf` | objdump |
| `mips64r5900el-ps2-elf-objdump` | Desassemblagem |
| Rev.053 | COP2 baseline (6 + 74 COP2 instructions) |
| Rev.054 | Handler convention (hC/hB/hA) |

## Findings

### 1. COP2 Cloth Functions — Decompilacao completa

#### clothSubDistanceCheck (0x1D3E80)

**Funcionamento:** Itera todos os state nodes de tipo 19, verifica distancia Euclidiana ao quadrado vs threshold, marca nos elegiveis como "ativos".

```
estado_atual = state_resolver(19)  // primeiro node type 19
enquanto estado_atual != NULL:
    diff.xyzw = estado.posicao - referencia.posicao
    diff.xyz  = diff.xyz * diff.xyz   // quadrado
    dist_sq   = diff.x + diff.y + diff.z  // soma dos quadrados
    se dist_sq < threshold_sq:
        [estado+0x48] >= 1? → skip  (so marca se state_id==0)
        [estado+0x08] != 0? → skip  (so marca se nao ocupado)
        senao:
            sub_19F530(estado)
            estado+0x74 = 0     // zera timeout/counter
            estado+0x08 = 1     // marca como ativo
    estado = state_next(estado)
retorna 1
```

**Dados:**
- 61 instrucoes, 9 basic blocks
- COP2: 5 VU (1 lqc2, vsub, vmul, vaddy, vaddz, 1 qmfc2)
- 4 jals: state_resolver(19), vec_load, sub_19F530, state_next

#### clothSubForceApply (0x1D3F78)

**Funcionamento:** Similar ao distance check mas aplica forca direcional ao inves de simples ativacao.

```
estado_atual = state_resolver(19)
enquanto estado_atual != NULL:
    angulo_rand = sub_118A68()           // valor aleatorio
    angulo_escalado = angulo_rand * 65536.0  // normaliza
    short_angulo = (short)(angulo_escalado)   // truncado para 16-bit
    senao ocupado:
        sub_19F530(estado)
        estado+0x74 = 0
        estado+0x08 = 1
    // aplica forca direcional:
    f_seno = sin(short_angulo)           // 0x10E950
    estado+0x130 = f13 * f_seno          // forca Y (f13 = direction magnitude)
    f_cosseno = cos(short_angulo)        // 0x118A68 (reusa)
    estado+0x134 = f12 * f_cosseno       // forca X (f12 = force magnitude)
    f_seno2 = sin(short_angulo)          // 0x10E9A0
    estado+0x138 = f13 * f_seno2         // forca Z
    estado = state_next(estado)
retorna 1
```

**Dados:**
- 74 instrucoes, 8 basic blocks
- 0 COP2 (sem VU macro-mode, usa funcoes math da EE: sin/cos)
- 6 jals: state_resolver(19), sub_118A68 (×2), sub_19F530, sub_10E950 (sin), sub_10E9A0 (sin)

**Nota:** A funcao `0x1D3F78` NAO contem COP2. Apenas `0x1D3E80` tem 5 COP2. A contagem de 6 COP2 da Rev.053 incluia `0x1D3E80` (5) + `0x1D3F78` (1, provavelmente um falso positivo na varredura anterior).

#### clothSubPlaneClip (0x1D45B0)

**Funcionamento:** Planejamento de recorte 3D completo com VU0. Recebe 2 vetores fonte, aplica transformacao 4x4, perspectiva, recorte contra frustum.

```
clothSubPlaneClip(dst_a, dst_b, src_a, src_b):
    // Monta matriz 4x4 a partir dos vetores fonte
    mat4x4_1(src_a + identity_w)    // empacota vec3 como vec4 com w=1
    mat4x4_2(src_b + identity_w)
    
    // Setup de transformacao
    t = setup_transform()
    mat_base = get_matrix_base(t)
    view_mat = setup_view_matrix(mat_base + 0x80)
    mul_mat4(m4_na_stack, mat4x4_1)
    mul_mat4(mat4x4_2_temp, mat4x4_2)
    
    // Z near-plane clip
    se view_z[1] < view_z[0]:  // ponto B atras de A?
        swap(dst_a, stack)
        swap(stack, dst_b)
        swap(dst_b, temp)
        s3 = 1 (swap_occurred)
    senao: s3 = 0
    
    se view_z[1] < 1.0:  // Z behind near plane?
        // Interpola linearmente usando COP2:
        vdiv Q = (1.0 - vf8.z) / (vf9.z - vf8.z)
        vf16.xy = vf9.xy - vf8.xy
        vf16.xy *= Q
        vf8.z = 0 + vf8.w
        vf8.xy += vf16.xy
        // guarda em dst_a
        s3 = 1 (clip occurred)
    
    // 4x4 transform + perspective divide (COP2 dense block)
    lqc2 vf8, (dst_a)     // carrega vertex A
    lqc2 vf9, (dst_b)     // carrega vertex B
    // Transforma A:
    vf10 = vf4*vf8.x + vf5*vf8.y + vf6*vf8.z + vf7*vf8.w
    // Transforma B:
    vf11 = vf4*vf9.x + vf5*vf9.y + vf6*vf9.z + vf7*vf9.w
    // Perspectiva divide:
    Q = vf0.w / vf10.w
    vf10.xyzw *= Q
    Q = vf0.w / vf11.w
    vf11.xyzw *= Q
    sqc2 vf10, (s1)     // guarda A transformado
    sqc2 vf11, (s0)     // guarda B transformado
    
    // X forward clip: se B.x < A.x → swap, marca
    // X reverse clip: interpola em X se A.x < -X_limit
    // Y forward clip: se B.y < A.y → swap, marca
    // Y reverse clip: interpola em Y se A.y < -Y_limit
    
    retorna s3:  -1 (culled), 0 (partial), 1 (fully visible)
```

**Dados:**
- 259 instrucoes, ~22 basic blocks
- **74 COP2 instructions** confirmados
- 7 jals: setup_transform, get_matrix_base, setup_view_matrix, mul_mat4 (×2), swap (×6)
- 6 blocos COP2: Z near-plane (10), 4x4+perspective (20), X forward (11), X reverse (11), Y forward (11), Y reverse (11)

**Wrappers:**

| Funcao | Endereco | Proposito |
|---|---|---|
| clothSubPlaneClipWrapper | 0x1D49C0 | Chama clip, copia coords, chama intersect/subdivide |
| clothClipAndRender | 0x1D4A58 | Setup viewport, chama clip, se parcial chama render intersect |
| clothClipMesh | 0x1D4A58+ | Wrapper final que integra clip + render |

### 2. BOY hC (0x1C1A98) — Constructor

| Handler | Endereco | Insns | Aloc | Tag | Jals | Papel |
|---|---|---|---|---|---|---|
| hC | 0x1C1A98 | ~108 | 76B | 0xFE (254) | 12 | Constructor: models + child entities + state init |

**Fluxo:**

1. **Alloc:** 0x13A0F8(gp_data, 76, 254) → alloc_ptr no heap (76 bytes)
2. **Store:** [entity_state + 0x800] = alloc_ptr
3. **5 model loads** via 0x1C8478:
   - 0x4CF7F0 c/ tex 0x4C01B0 → state+0x18 (mesh A)
   - 0x4CFAF0 c/ tex 0x4C0270 → state+0x1C (mesh B)
   - 0x4CFDF0 s/ tex → state+0x24 (mesh C)
   - 0x4CFF30 s/ tex → state+0x20 (mesh D)
   - 0x4C0070 s/ tex → state+0x28 (mesh E)
4. **[entity_state + 0x554] = 1** (flag de inicializacao)
5. **3 child entities** via 0x19F310(type, init):
   - type 1 → state+0x04
   - type 11 → state+0x08
   - type 12 → state+0x0C
6. **Transform setup:** 0x243BD8(child+0x20, ...)
7. **entity_state_reg**(a1=0, a2=1216, a3=0, t0=12, t1=0)
   - **Notavel:** a1=0, a3=0, t1=0 — ao contrario de outras entidades que usam valores altos (1216/1820/514)
8. **0x1C1380**(entity, initializer) — init especifico do BOY
9. **descriptor_setup**(entity, 2)
10. **Init state fields:**
    - state+0x10 = 0 (contador)
    - state+0x14 = 0
    - state+0x2C = 20 (config A)
    - state+0x30 = 20 (config B)
    - state+0x34 = 300.0f (alcance A)
    - state+0x38 = 300.0f (alcance B)
    - state+0x48 = 0x80808080 (flag mask)
11. **sub_10C6C0**(state+0x2C) — init do config block
12. **Retorna** alloc_ptr

### 3. BOY sub-function (0x1C1C48) — Distance trigger

Chamada por BOY hB (posicao 3 na sequencia de 5 chamadas).

- Acessa gp_data (contexto do jogador)
- Compara entity_state+0x4A0 (posicao) com gp_data+0x4A0
- XOR mask matching contra tabela em 0x4D4150 (stride 8, 6 entries)
- Se match: sub_1184B8 (distance vector), sub_1183B8 (transform), vec_load
- Se distancia < 10.0f (0x4120):
  - sub_1118B8(11) — som/evento
  - sub_111FA8(1, 5, 128) — config
  - Transform setup + matrix init
  - Matrix upload + sub_11E220(0x4C0330, 16, 8) — render setup
  - Segundo matrix upload + sub_11E220(0x4C0340, 16, 8)
  - sub_10F630() — finalizacao

### 4. hB Dispatcher Investigation

O gap do dispatcher de hB (quem chama descriptor[+0x50]) continua **nao resolvido**.

**O que foi tentado:**

1. Busca por `lw ... 0x50` seguido de `jalr` em todo .text: **inconclusivo** — 40+ loads em 0x50, a maioria stack ou struct field, nenhum claramente apontando para descriptor table
2. Busca por `lui ... 0x002A` (descriptor table base): 100+ ocorrencias em funcoes de inicializacao/registro, nenhuma em loop de update
3. Busca manual em 0x1B76F8 (entry iteration): chama hC (+0x58), nao hB (+0x50)

**Hipoteses restantes:**

| Hipótese | Confianca | Evidencia |
|---|---|---|
| Node system dispatcher (0x13F9D0/0x13FB70) | Media | Callbacks registrados passam por este sistema. Mas 0x1D3A30 nunca foi registrado. |
| Loop interno do proprio descriptor (self-dispatch) | Baixa | Nenhuma funcao cloth le proprio +0x50 |
| Cutscene/event system | Media | 0 hits em 90 min de gameplay. Pode ser chamado apenas em cutscenes. |
| VU0 interrupt callback | Media-Baixa | hB contem COP2, mas nenhum mecanismo de callback VU0→EE foi encontrado |
| Dead code | Baixa | Funcao tem 4 COP2 + logica complexa. Improvavel que nunca seja usada. |

**Proximo passo recomendado:** Breakpoint em WOODBOX0 hB (0x1C0538) ou ENEMY1 hB (0x1CE3C0) — handlers que SABEMOS ser per-frame — para capturar a pilha de chamadas e identificar o dispatcher real.

### 5. Status dos handlers pendentes

| Entidade | hC | hB | hA | init_fn | Status |
|---|---|---|---|---|---|
| BOY (1) | **0x1C1A98** ✅ (108 insns) | 0x1C1DD8 | 0x1C1F58 | 0x153478 | **Completo** |
| GIRL (2) | 0x1D1668 | 0x1D17F8 | 0x1D1A98 | 0x174BA0 | Completo (Rev.054) |
| ENEMY1 (4) | 0x1CE220 | 0x1CE3C0 | 0x1CE690 | 0x164440 | Completo (Rev.053) |
| WOODBOX0 (17) | 0x1C00C0 | 0x1C0538 | 0x1C05D0 | 0x17D1D0 | Completo (Rev.053) |
| BARREL (19) | 0x1D27A8 | 0x1D3A30 | 0x1D3B28 | 0 | Completo |
| BGA (30) | 0 | 0 | 0 | 0x203EE8 | Completo (Rev.054) |
| BIRD (32) | ? | ? | ? | 0x1971C0 | **Pendente** |
| QUEEN (46) | 0x19A7E8 | 0x19A8F0 | 0x19A9A0 | 0x19B7F8 | Completo (Rev.054) |
| DEVIL_GI (48) | ? | ? | ? | 0x174BA0 | **Pendente** (compartilha GIRL model) |
| AP1 (61) | 0x1B8720 | 0x1BA330 | 0x1BA530 | 0x1BB6B0 | Completo (Rev.054) |
| ATTACKCH (62) | ? | ? | ? | 0x1BBF78 | **Pendente** |
| ATTACKCH (63) | ? | ? | ? | 0x1BBF78 | **Pendente** |
| BOSS_CTR (64) | ? | ? | ? | 0x198140 | **Pendente** |

## What is Confirmed

- clothSubDistanceCheck (0x1D3E80): proximity wake-up para cloth nodes type 19, 5 COP2
- clothSubForceApply (0x1D3F78): aplicacao de forca direcional, sin/cos via EE, 0 COP2 (corrigido)
- clothSubPlaneClip (0x1D45B0): 4x4 transform + perspective divide + frustum clip, 74 COP2
- BOY hC: 76B alloc (tag 254), 5 models, 3 child entities, entity_state_reg com parametros zero
- BOY sub-function 0x1C1C48: distance trigger com XOR mask matching
- ATTACKCH, BOSS_CTR, DEVIL_GI, BIRD: handlers pendentes

## What is Unknown

- Dispatcher de hB: continua sem solucao
- Conteudo das funcoes ATTACKCH, BOSS_CTR, DEVIL_GI, BIRD
- Semantica dos parametros entity_state_reg (a2=1216 onipresente, a1/a3/t0/t1 variam por entidade)

## Next Minimum Test

1. Breakpoint em WOODBOX0 hB (0x1C0538) para capturar call stack do dispatcher de hB
2. Disassemblar ATTACKCH (62/63) e BOSS_CTR (64) — fechar a tabela de 12 init_fn
3. Disassemblar BOY init_fn (0x153478) — unico init_fn de protagonista pendente

## Conservative Verdict

As duas funcoes COP2 reais do sistema cloth foram decompiladas: uma e um wake-up por proximidade (5 COP2), a outra e um transform+clip completo (74 COP2). A terceira funcao identificada na Rev.053 como "COP2" (0x1D3F78) nao contem COP2 — e uma aplicacao de forca direcional via sin/cos EE. BOY hC completa o conjunto de handlers dos protagonistas (BOY, GIRL). O dispatcher de hB permanece o maior gap do projeto — requer runtime breakpoint num handler per-frame confirmado (WOODBOX0 ou ENEMY1) para rastrear a cadeia de chamadas.
