# Rev.141 — Consumidor USA de path confirmado: `jal 0x265024` = `strcmp` (44 callers); bug do decoder JAL corrigido

- **Date:** 2026-09-06
- **Branch:** native-port
- **Trilha:** PORT (P2 — acesso a DATA.DF) / estática
- **Objective:** fechar o passo 2 do "próximo mínimo teste" da Rev.140 — "procurar no ELF USA o consumidor que faz lookup de path (strncmp `jal 0x265024`)" — e, no processo, corrigir uma conclusão falsa registrada na sessão anterior ("0 JAL refs para 0x265024").

---

## Correção crítica — bug no decoder de JAL da sessão anterior

A sessão anterior reportou **0 callers diretos** para `0x265024`, contradizendo a Rev.138 ("faz lookup de nome via `jal 0x265024` (equivalente strncmp)"). A contradição era um **bug do meu decoder de JAL**:

```python
t = (w & 0x03FFFFFF) | (base & 0xF0000000)     # ERRADO
t = ((w & 0x03FFFFFF) << 2) | ((pc + 4) & 0xF0000000)   # CORRETO
```

O campo de 26 bits do JAL é o alvo em palavras (`target >> 2`), não um offset absoluto. `0x0C099409` em `0x132854` → imm26 = `0x99409` → `imm26 << 2` = **`0x265024`**. O upper é `0x10000000` (alta de `PC+4`). Com o decoder corrigido:

- `jal 0x265024` tem **44 callers** reais no `.text`.
- **Confirma a Rev.138**: o loop de lookup do índice 48B do reader DFDATAS chama exatamente `0x265024` em `0x132854`.

## Identidade de `0x265024` = `strcmp` (não `strncmp`)

A função em `0x265020-0x265424` é uma **strcmp otimizada de 64 bits** (padrão haszero):

- Prologue: `addiu $sp,sp,-0x20` (0x265020) → `or $t0,$a0,$a1` + `andi $v0,$t0,7` (0x265028, checagem de alinhamento em 8).
- Constantes clássicas `0x0101010101010101` (`$a3`, via lui/ori/dsll/ori ×2) e `0x8080808080808080` (`$a2`).
- `$a2` é **clobbered com a constante de máscara** — ou seja, **não pode** ser o limite de contagem de um `strncmp`. A função não lê um registrador de comprimento: é **`strcmp` puro** (retorna 0 se igual).
- Caller `0x132854` usa `beqz $v0` → achou → segue para o processamento do registro.

**Correção de terminologia:** `0x265024` na Rev.138 deve ser lido como **`strcmp`** (retorna diferença / 0), não `strncmp`. Sem impacto na análise: o lookup de nome no índice 48B usa strcmp sobre path montado (`DFDATAS/%s`).

## Cadeia de acesso confirmada no reader open (0x132630)

Re-exame com decode JAL correto:

| Endereço | Instrução | Significado |
|----------|-----------|-------------|
| 0x132630 | `addiu $sp,$sp,-0x2c0` | funções C/reader open |
| 0x132680-0x132694 | argumentos a0..t1 salvos; t2/t3 em 0x208/0x20C($sp) | 4 handlers + 2 extras do chamador |
| 0x132698-0x1326F4 | primeiras 7 slots de 0x6A5C80 | thread table pré-open |
| 0x1326F8 / 0x132874 | `lui s0,0x55; addiu s0,s0,0x6818` → `ios/cdvd.c` + asserts | função usa ios/cdvd.c |
| 0x1327A0-0x1327B0 | `DFDATAS/%s` (0x556A10) via `lui 0x55; addiu 0x6a10` → `sprintf` | monta path completo |
| 0x1327C0-0x1327C8 | `lui 0x63; addiu 0x1f70` → `\%s;1` | variante com versão ISO |
| 0x132824-0x13282C | `jal 0x265168` (strcpy-like) | copia nome da entrada |
| 0x132848 | `lui s0,0x28; addiu v0,s0,-0x1ad8` → **0x27E528** | base do índice 48B em RAM |
| 0x13284C-0x132858 | `mult $ac2,$s1,$a1` (s1=idx, a1=0x30) + `jal 0x265024` | combo: nome da entrada i vs path → **strcmp** |
| 0x13285C | `beqz $v0, 0x13289c` | match → processa registro |
| 0x132860-0x13286C | `lw v0,-0x699c($gp)`; `slt $v0,$s1,$v0`; `bnel` | loop até contador do índice |

Datas de registro (estrutura entry 48B configurada por `0x132630`):
- `entry+0x100` = nome (`0x265730` ret).  `entry+0x104` = handler a2 (`sw $s5,0x104`). `entry+0x108` = flags/tamanho. `entry+0x114/0x118/0x11C/0x120` = handlers/tamanhos registrados. Coerente com o bloco de captura Rev.138.

## Dispatch dos handlers de leitura (direto vs inflate) — estado

Identificados os 2 sites de `jalr` que despacham os handlers armazenados:

- **0x1329F4**: `jalr $v0`, com `$v0 = entry+0x118` (handler a3); `beqz` em 0x1329E0 pula se nulo. Args: a0=entry, a1=entry+0x120, a2=`srl(entry+0x108,2)&1`.
- **0x132B0C**: `jalr $v1`, com `$v1 = entry+0x11C` (handler t0); `beqz` em 0x132B00 pula se nulo.

Callers do open (0x132630) e os handlers que passam:

| Caller (VA) | Função-envelope | a3 handler | t0 handler | Observação |
|---|---|---|---|---|
| 0x13857C / 0x138684 | 0x13850C / 0x138614 | t1=t2=t3=0 | t1..=0 | a2=buffer; nenhum handler |
| 0x1406C0 | 0x140338 | `0x141128` (**stub `jr $ra`**) | `0x141130` (**stub `jr $ra`**) | a1=`0x141048` (read real) |
| 0x140840 | 0x140708 | `0x141160` (**stub `jr $ra`**) | 0 | a1 handler real escrito em a1 |
| 0x176738 | 0x1765F8 | 0 | 0 | a2=`s4` |
| 0x19FC1C | 0x19FB2C | `0x1A0090` (stub tiny) | 0 | a2=0 |
| 0x1F0F50 | 0x1F0E38 | 0 | 0 | a2=0 |

O único handler de leitura real observado é **`0x141048`**, que efetua `x = mt_ioRead(...)` (0x25DFB0), `seek`/`read` via `0x132DC0` (raw) e `0x133500` (chunked). Os demais slots (`0x141128/0x141130/0x141160`) são **stubs `jr $ra`** — "não transforma / sem erro".

`ios/inflate.c` (0x556B00) está referenciado em `0x135A54` (assert do próprio inflate), logo o **código inflate existe no USA**, mas não aparece como handler nos callers do open DFDATAS analisados.

## Refinamento "direto vs transformado" (USA)

- **Confirmado**: lookup de path = `strcmp` (0x265024), 44 callers; reader open constrói `DFDATAS/%s` (0x556A10) e `\%s;1` (0x631F70); índice 48B na RAM (0x27E528, stride 0x30, contador gp-0x699C).
- **Confirmado**: inflate existe no USA (refs 0x135A54/0x135380-0x13539C).
- **Provável**: os callers do open que passam `0x141128/0x141130/0x141160` (stubs) estão tratando o arquivo como **não-comprimido puro** (handler de transformação no-op), e `0x141048` (read raw via 0x132DC0/0x133500) serve o buffer direto.
- **Provável**: a seleção inflate-vs-plain é feita **pelo chamador** no momento do open (handlers em a2/a3/t0/t1), não por inspeção de conteúdo no reader — consistente com a Rev.138.
- **Unknown**: o caller exato que abre os estágios (ex.: STGST00A.DF) e se esse fluxo passa handler inflate (0x135A48/0x135AD8) ou stub. Os 7 callers de open estão mapeados (funcs 0x13850C/0x138614/0x140338/0x140708/0x1765F8/0x19FB2C/0x1F0E38); ainda não nomeados por símbolos Ghidra.

## Confirmado / provável / possível / desconhecido / descartado

- **Confirmado**: `jal 0x265024` existe (44 callers); identidade `strcmp` (a2 é máscara, não limite); chamado em 0x132854 para lookup do índice 48B; base 0x27E528 confirmada por `lui 0x28 + addiu -0x1ad8`; "0 callers" da sessão anterior = bug do decoder JAL, não fato.
- **Provável**: payload USA consumido direto nos callers com handlers-stub; seleção inflate decide-se no chamador.
- **Possível**: algum caller de estágio usa handler inflate real, explicando a alta entropia dos `.DF` USA (0xFF/0x7F-heavy) como stream comprimido com formatless flag em TOC (não byte0).
- **Unknown**: nome simbólico das 7 funções-envelope; VA do fluxo que abre estágios especificamente.
- **Descartado**: "0x265024 sem callers"; "strncmp com limite" (é strcmp).

## Próximo mínimo teste

1. Nomear as 7 funções-envelope dos callers de open (0x132630) via símbolos Ghidra/PAL (0x13850C…0x1F0E38).
2. Identificar qual caller abre arquivos de estágio (STRINGS `STGST%A.DF`, e o argumento a1/2x204) e se passa handler inflate ou stub — isso fecha "direto vs transformado" para os `.DF` USA.
3. (Runtime, não autorizado) memdump do buffer pós-handler vs blob on-disc.

## Verdict conservador

A contradição registrada na sessão anterior foi resolvida como bug do decoder JAL; a Rev.138 permanece válida com uma correção de terminologia (`strncmp`→`strcmp`). O consumidor de path USA está confirmado (44 callers, incluindo o reader DFDATAS). A decisão direto-vs-inflate para os `.DF` USA permanece dependente de nomear os callers de open de estágio.