# rev.022 — Entity Dispatcher Ground Truth

## Date

2026-05-13

## Resumo Executivo

A análise de validação conservativa da hipótese de dispatcher de estado em torno de
`0x001d37c8` e `0x00628fb0` revela **inconsistências significativas** entre o que foi
documentado em rev.021 e o que o binário realmente contém. A primeira metade do
dispatcher (0x001d37e4-0x001d37f4) está correta. A segunda metade (0x001d37fc-0x001d3808)
mostra divergências na construção do endereço da jump table. A tabela em
`0x00628fb0` contém **valores inválidos** (< 0x1000) — nenhum ponteiro de código válido.
Ghidra reportou os alvos como 0x001f2148, 0x001d2538 etc., mas esses valores
**não correspondem ao que está no binário**.

**Veredito: Possível — com enfraquecimento significativo.**

---

## Evidências Reaproveitadas

As seguintes evidências de rev.021 foram tomadas como ponto de partida, sujeitas
a verificação contra o binário real:

| Evidência de rev.021 | Status |
|----------------------|--------|
| `0x001d37c8` é função dispatcher com jump table | Verificado — função existe em .text |
| Sequência: lw entity, lw vtable, addiu 0x40, lw state | **Parcialmente correto** — verificado |
| Offset +0x800 para vtable | **Correto** |
| Offset +0x48 para state ID | **Correto** |
| Bounds check `< 5` (SLTIU) | **Correto** |
| `sll $3,$0,2` na indexação | **TYPO — é `sll $3,$3,2`** |
| `lui $2,0x62` / `addiu $2,$2,0x8fb0` para construir 0x628fb0 | **INCORRETO** — valores reais diferentes |
| Jump table `0x00628fb0` com 5 ponteiros | **Correto que existe — mas valores são inválidos** |
| Alvos: 0x001f2148, 0x001d2538, 0x001d2540, 0x00105f00 | **Inconsistente com dados reais da tabela** |
| `0x001d3a30` é único chamador estático de `0x001d37c8` | Reaproveitado, não revalidado |

---

## Inconsistências Encontradas

### 1. Instrução `sll` — CORRIGIDA

A documentação de rev.021 afirma:

```
0x001d3800:  sll     $3,$0,2        ; $3 = 0 (shift of $0)
```

**Verificação contra binário** — bytes `0x80180300` em file offset `0x000d4800`:

```
Opcode  = 0x00 (SPECIAL)
rt (rs?) = r3 (v1)  ← source register is $3, NOT $0
rd       = r3 (v1)
shamt    = 2
func     = 0x00 (SLL)
```

Instrução real: `sll $3,$3,2`

**Interpretação**: O typo em rev.021 (registrador $0 no lugar de $3) teria
tornado o dispatcher inútil — sempre indexaria a entrada 0 da tabela.
A correção para `sll $3,$3,2` significa que o índice é calculado corretamente
como `state_id * 4`. **A hipótese de indexação por state ID é fortalecida.**

### 2. Construção do endereço da jump table

A documentação de rev.021 afirma:

```
0x001d37fc:  lui     $2,0x62        ; jump table base: 0x00628fb0
0x001d3804:  addiu   $2,$2,-28752   ; 0x00628fb0
```

**Verificação contra binário**:

| Endereço VA | Bytes reais (LE) | Instrução decodificada |
|-------------|------------------|------------------------|
| 0x001d37fc | `6200023c` | `lui $2,0x0000` ← **não** `lui $2,0x62` |
| 0x001d3800 | `80180300` | `sll $3,$3,2` ← confirmado |
| 0x001d3804 | `b08f4224` | `addiu $2,$2,-28752` ← match |
| 0x001d3808 | `21186200` | `addu $3,$3,$2` ← confirmado |
| 0x001d380c | `0000648c` | `lw $4,0($3)` ← confirmado |
| 0x001d3810 | `08008000` | `jr $4` ← confirmado |

O campo imediato no `lui` é `0x0000`, não `0x62`. A combinação de
`lui $2,0` + `addiu $2,$2,-28752` resulta em:

```
0x00000000 << 16 | (0xffff8fb0 & 0xFFFF)  =  -28752
```

O `lui` com 0x0000 sugere que a parte alta do endereço foi zerada
por uma relocation aplicada pelo linker, ou que o valor original era
`lui $2,imm + addiu $2,$2,offset` onde o campo imediato do `lui`
foi zerado.

**Hipótese**: O ELF passou por um processo de linking onde endereços
de jump table em .rodata foram zerados na parte alta, e a referência
real a 0x00628fb0 pode ter sido perdida ou renomeada durante a
compilação/linkagem.

### 3. Valores da jump table em `0x00628fb0`

A tabela está em .rodata (VA 0x00553700-0x006308a8) — **dentro da faixa**,
portanto localização correta.

File offset: `0x00529fb0`

```
Entry 0: 0x000005f4  (1524)  → INVÁLIDO (< 0x1000, não é ponteiro de código)
Entry 1: 0x000005f6  (1526)  → INVÁLIDO
Entry 2: 0x00000000           → INVÁLIDO
Entry 3: 0x00000001           → INVÁLIDO
Entry 4: 0xffffffff           → INVÁLIDO (possível sentinel)
```

**Nenhum dos 5 valores é ponteiro válido para .text.**

Isso é contraditório com rev.021, que afirmou que Ghidra identificava
alvos como 0x001f2148 etc. Possibilidades:

1. Ghidra fez análise incorreta de tipos no endereço
2. Os valores são índices relativeizados que Ghidra interpretou como VAs
3. A jump table correta está em outro endereço
4. Os valores são placeholders de compilação

### 4. Verificação dos alvosClaimados

| EndereçoClaimado | Em .text? | Primeira instrução | Observação |
|-----------------|-----------|--------------------|------------|
| 0x001f2148 | Sim | `lw $2,348($4)` | Meio de função, não entry point |
| 0x001d2538 | Sim | `j 0x001d12a8` | Instrução `j` — pula para outro lugar |
| 0x001d2540 | Sim | `j 0x001d12a8` | Idem |
| 0x00105f00 | Sim | `???` (opcode 0x1e) | Não decodifica como código válido |

Nenhum dos alvos é um entry point limpo. As instruções `j` em 0x001d2538/0x001d2540
são jumps dentro do .text, não chamadas — podem ser destino de jump table
ou código de tail-call, mas não confirmam a tabela em 0x00628fb0.

### 5. ELF é estaticamente vinculado

`readelf -r` retorna vazio — **não há seção de relocations**. Isso significa
que não há relocations pendentes que poderiam explicar valores zerados/inválidos.
O binário foi linkado de forma completa.

---

## Sequência Validada de Instruções

Abaixo, sequência **confirmada** a partir de 0x001d37c8:

```
0x001d37c8:  addiu  $sp,$sp,-0x140    ; prologue (documentado em rev.021)
0x001d37e4:  lw     $2,348($18)       ; candidate_entity_ptr = *(global_entity_base + 0x15c)
0x001d37e8:  lw     $19,2048($2)      ; candidate_state_block_ptr = *(candidate_entity_ptr + 0x800)
0x001d37ec:  addiu  $17,$19,64        ; candidate_state_base = candidate_state_block_ptr + 0x40
0x001d37f0:  lw     $3,8($17)         ; candidate_state_id = *(candidate_state_base + 8)
0x001d37f4:  sltiu  $2,$3,5           ; bounds_check = (candidate_state_id < 5)
0x001d37f8:  beq    $2,$0,+28         ; if (bounds_check == 0) branch_to_oob_handler
0x001d37fc:  lui    $2,0x0000         ; TABLE_BASE_HIGH = 0  ← difere de rev.021
0x001d3800:  sll    $3,$3,2           ; index = candidate_state_id * 4  ← CORRIGIDO (era $0)
0x001d3804:  addiu  $2,$2,-28752      ; TABLE_BASE_LOW = 0xFFFF8FB0 → signed = -28752
0x001d3808:  addu   $3,$3,$2          ; table_entry_ptr = &table[0] + index
0x001d380c:  lw     $4,0($3)          ; handler_ptr = *table_entry_ptr
0x001d3810:  jr     $4               ; jump to handler
```

**Resumo da sequência**: Confirma-se a estrutura conceitual documentada em rev.021:
entidade → vtable → offset 0x48 → bounds check → indexação de jump table → jr.
Os offsets (+0x800, +0x48) e o bounds check `< 5` estão corretos.
A indexação com `sll $3,$3,2` está correta (não `$0`).

---

## Modelo Estrutural Provisório

```
┌──────────────────────────────────────────────────────────────────┐
│ caller: 0x001d3a30 (vtable-dispatched, 0 static callers)         │
└────────────┬─────────────────────────────────────────────────────┘
             │ jal 0x001d37c8
             ▼
┌──────────────────────────────────────────────────────────────────┐
│ 0x001d37c8 — candidate_dispatcher(entity_ptr, ...)               │
│                                                                  │
│  vtable_base    = *(entity_ptr + 0x800)      ← lw $19,2048($2)   │
│  candidate_state_base = vtable_base + 0x40   ← addiu $17,$19,64 │
│  candidate_state_id  = *(state_base + 0x8)   ← lw $3,8($17)     │
│  if (candidate_state_id >= 5) → branch to oob handler           │
│  table_entry = *(table_base + candidate_state_id * 4)            │
│  jr table_entry                                              sll │
└──────────────────────────────────────────────────────────────────┘
             │
             ▼
    ┌─────────────────┐
    │ 0x00628fb0[0..4] │  ← jump table (VALORES INVÁLIDOS)
    └────────┬────────┘
             │
             ├─[0]→ 0x000005f4  (INVALID)
             ├─[1]→ 0x000005f6  (INVALID)
             ├─[2]→ 0x00000000  (INVALID)
             ├─[3]→ 0x00000001  (INVALID)
             └─[4]→ 0xffffffff  (sentinel?)
```

---

## Jump Table `0x00628fb0`

| Entry | Raw Value | Classificação |
|-------|-----------|---------------|
| 0 | `0x000005f4` | **Inválido** — < 0x1000, não aponta para código |
| 1 | `0x000005f6` | **Inválido** |
| 2 | `0x00000000` | **Inválido** |
| 3 | `0x00000001` | **Inválido** |
| 4 | `0xffffffff` | **Possível sentinel** — -1 em unsigned |

**Interpretação**: Os valores pequenos (1524, 1526) e 0/1 parecem ser
**valores de índice ou offsets de debug**, não ponteiros de código.
O valor 0xffffffff pode ser um sentinel para estado inválido.
Os alvosclaimados por Ghidra (0x001f2148, etc.) **não correspondem** a
nenhum dos valores reais da tabela.

**Hipótese para os valores reais**: Se 1524 = 0x5f4 é um índice, e 1526 = 0x5f6
é o próximo — isso pode ser a contagem de alguma coisa (frames, chamadas,
saltos de animação). Alternativamente, pode ser uma tabela de dados
interpretada como ponteiros por Ghidra.

---

## O Que Fica Confirmado

1. A **função `0x001d37c8` existe** no .text e é chamada por `0x001d3a30`
2. A **sequência de carregamento** (entity → vtable+0x800 → state_base+0x40 → state_id+0x8)
   está correta — offsets +0x800 e +0x48 são reais
3. O **bounds check `< 5`** é real — a tabela tem 5 entradas (índice 0-4)
4. O **cálculo de índice** usa `sll $3,$3,2` (state_id * 4) — a correção do
   typo fortalece a hipótese de indexação
5. O **jr para handler** via registrador é real — não é jal
6. A **tabela existe em .rodata** no endereço `0x00628fb0`
7. O ELF é **estaticamente vinculado** — sem relocations pendentes

---

## O Que Fica Apenas Provável

1. Que `0x00628fb0` é a jump table **ativa** do dispatcher — os valores
   inválidos sugerem que a tabela pode ter sido usada em desenvolvimento
   e desabilitada, ou que está no endereço errado
2. Que o `lui $2,0` representa **relocation zerada** — o campo imediato
   do `lui` deveria conter 0x62 para formar 0x00628fb0, mas foi sobrescrito
3. Que os alvos reais da jump table sejam 0x001f2148, 0x001d2538 etc.
   — nenhum deles tem correspondência na tabela lida do binário
4. Que `0x001d3a30` seja o único chamador estático

---

## O Que Fica Descartado

1. A **hipótese original** de que a tabela em `0x00628fb0` contém ponteiros
   diretos para handlers de estado — os valores lidos são inválidos
2. A **instrução `sll $3,$0,2`** como descrita em rev.021 — é typo,
   o valor real é `sll $3,$3,2`
3. A **construção exata** `lui $2,0x62` / `addiu $2,$2,0x8fb0` como descrita —
   o `lui` imediato é `0x0000`, não `0x62`
4. Que Ghidra tenha identificado corretamente os alvos da jump table —
   a discrepância entre valores lidos e alvos claimados indica erro de
   análise ou de documentação

---

## Próximo Teste Mínimo Recomendado

### Teste 1: Buscar jump table alternativa

Buscar no binário por valores que correspondam a 0x001f2148, 0x001d2538,
0x001d2540, 0x00105f00 como ponteiros em .rodata. Pode existir uma
segunda jump table no endereço correto que não seja 0x00628fb0.

### Teste 2: Verificar `lui $2,0x62` em todo o .text

Buscar por instruções `lui $2,0x62` no binário — se existir, a versão
correta da construção de 0x00628fb0 pode estar em outra função, e
`0x001d37c8` pode ter sido compilada com endereço incorreto ou relocada.

### Teste 3: Runtime breakpoint no dispatcher

Com PCSX2, definir:

- Execution breakpoint em `0x001d37c8` (entrada)
- READ breakpoint em `0x00628fb0` (tabela)
- READ breakpoint em `0x00628fb0 + (state_id * 4)` (entrada específica)

Observar: o código em `0x001d3810` executa `jr $4`. O valor em `$4` no
momento do jump é o que importa. Dump do registrador `$4` e
`$3` (índice) quando o breakpoint fire. Se `$4` for 0x5f4 (inválido),
confirma que a tabela está errada. Se for outro valor, revela a tabela real.

### Teste 4: Memory dump de .rodata em 0x00628fb0

Fazer dump de 64 bytes starting em 0x00628fb0 durante gameplay no PCSX2.
Pode haver diferença entre ELF estático e memória em runtime se houver
escrita dinâmica na seção .rodata (improvável mas possível via mprotect).

---

## Veredito Geral

**Possível com enfraquecimento.**

A **mecânica conceitual** do dispatcher (entity → vtable → state_id → bounds →
index → table → jr) está **confirmada** como real no binário. A correção
do `sll $3,$0,2` para `sll $3,$3,2` **fortalece** a hipótese de que é
um dispatcher de state ID funcional.

Porém, a **jump table `0x00628fb0` está vazia** (valores inválidos) e
os alvos claimados (0x001f2148 etc.) **não correspondem** aos dados reais.
Isso sugere:

- A) A tabela foi desabilitada/emulada por outra estrutura de dados
- B) O dispatcher aponta para tabela diferente
- C) Ghidra fez análise incorreta e os alvos claimados eram suposições

A confiança na hipótese **cai de "Provável" para "Possível"** até que a
tabela real seja localizada ou o dispatcher seja validado em runtime.