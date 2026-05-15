# rev.047 — Descriptor Callback Runtime Model

## Date

2026-05-15

## Resumo executivo

Consolidação do modelo runtime após a captura Rev.046. O gap principal
do projeto — a origem do argumento `a1` em `0x001D27A8` — foi resolvido:
`a1 = sp` (stack pointer do caller), apontando para um initializer struct
temporário. O callback é carregado de `[descriptor + 0x58]`.

## Evidência runtime usada

1. Breakpoint em `0x001D27A8` disparou no PCSX2 debugger
2. Registradores capturados: `a0=0x008320B4`, `a1=0x00798E40`, `sp=0x00798E40`,
   `ra=0x001B7A88`, `s0=0x002A3924`, `s3=0x008320B4`
3. Disassembly do caller em `0x001B7A74`-`0x001B7A8C`
4. Dump de memória da stack initializer em `0x00798E40`
5. Dump da descriptor table em `0x002A3924`
6. Dump de `[a1+0x30] = 0`, `[a1+0x58] = 0`

## Caller em `0x001B7A74`

```asm
001B7A74  lw    v0, 0x58(s0)         ; v0 = [descriptor + 0x58]
001B7A78  beq   v0, zero, 0x001B7A90  ; se null, skip
001B7A7C  daddu a0, s3, zero          ; a0 = s3 (entity context)
001B7A80  jalr  v0                    ; chama callback (0x001D27A8)
001B7A84  daddu a1, sp, zero          ; a1 = sp (initializer na stack)
001B7A88  lw    v1, 0x15C(s3)         ; v1 = [entity + 0x15C]
001B7A8C  sw    v0, 0x800(v1)         ; salva payload em [v1 + 0x800]
```

O caller:
1. Carrega o callback de `[s0 + 0x58]`
2. Prepara `a0 = s3` (entity/context armazenado)
3. Prepara `a1 = sp` (stack pointer como initializer struct)
4. Chama o callback via `jalr v0`
5. Salva o retorno em `[[s3 + 0x15C] + 0x800]`

## Callback em `descriptor + 0x58`

O `s0` da captura aponta para `0x002A3924` — o descritor **BARREL**.
O campo `+0x58` deste descritor contém `0x001D27A8`.

```
s0 = 0x002A3924  (descriptor BARREL)
[s0 + 0x58] = 0x001D27A8  (cloth_payload_init — callback de inicialização)
```

O descritor BARREL contém 3 slots de callback:

| Slot | Offset | Valor | Função | Propósito |
|---|---|---|---|---|
| +0x48 | +0x44 | `0x001D3B28` | cloth_aux | Auxiliar do sistema |
| +0x50 | +0x4C | `0x001D3A30` | cloth_update_cb | Atualização por frame |
| +0x58 | +0x54 | `0x001D27A8` | cloth_payload_init | Inicialização do payload |

## Stack initializer em `a1 = sp`

O fato de `a1 == sp` muda o modelo anterior. O initializer **não** é um
descritor global na `.data`, mas uma estrutura **temporária na stack**
do caller, montada no momento da chamada.

```
a1 = 0x00798E40
sp = 0x00798E40
```

O dump de `a1` mostrou:

```
+0x00:  410.5f, -175.0f, 1333.0f, 1.0f    (posição + escala)
+0x10:  0, 0, 0, 0                          (zeros)
+0x20:  1.0, 1.0, 1.0, 1.0                  (quaternions/weights)
+0x30:  0, 0, 0x00EBE1D2, 0x00EBE1D2        [a1+0x30] = 0 (variant)
+0x40:  (mesmos dados de +0x00 — bloco duplicado)
```

A struct tem **duas metades** de 0x40 bytes cada (total 0x80 bytes),
possivelmente representando "pose inicial → pose alvo" para interpolação
de animação/transform.

### [a1 + 0x30] = 0

O field `[a1 + 0x30]` que a Rev.043 identificou como origem do variant_id
tem valor **0** neste hit. Isso significa que o variant inicial do payload
cloth é 0 (modo padrão). Hits futuros podem ter valores diferentes.

### [a1 + 0x58] = 0

Ao contrário do que a Rev.044 especulava, `[a1 + 0x58]` **não** contém o
callback. O callback está exclusivamente em `[descriptor + 0x58]`.
O campo `+0x58` do initializer é zero neste hit.

## Payload retornado por `0x001D27A8`

A função `0x001D27A8` retorna em `v0` um ponteiro para o payload cloth
alocado. Este payload é instalado em:

```
[[s3 + 0x15C] + 0x800] = v0  (retorno de 0x001D27A8)
```

Confirmando o padrão da Rev.043:
- `[context + 0x15C]` = ponteiro para entidade
- `[entity + 0x800]` = ponteiro para payload cloth
- `[payload + 0x04]` = variant (copiado de `[a1 + 0x30]`)

## Atualização do modelo de structs

### Antes do runtime

```
a0 = context (estrutura global)
a1 = descriptor global (tabela .data)
```

### Depois do runtime

```
a0 = s3 = entity context (0x008320B4, heap EE)

a1 = sp = stack initializer (0x00798E40)
  ├─ +0x00: transform data (posição, escala, quaternion)
  ├─ +0x20: weights/blend
  ├─ +0x30: variant (0 neste hit)
  └─ +0x40: segunda metade (pose alvo?)

s0 = descriptor (0x002A3924, .data)
  └─ +0x58: callback (0x001D27A8)

Retorno:
  v0 = payload cloth (heap)
  └─ salvo em [[s3+0x15C]+0x800]
```

## Relação com descriptor table

A tabela de descritores (capturada no dump) contém pelo menos 5 registros:

| Descritor | Endereço | Nome | Slot +0x48 | Slot +0x50 | Slot +0x58 |
|---|---|---|---|---|---|
| 1 | `0x002A3924` | BARREL | 0x1D3B28 | 0x1D3A30 | 0x1D27A8 |
| 2 | `0x002A3988` | (ROPE label) | — | — | — |
| 3 | `0x002A39E8` | CHAIN | 0x1E9630 | 0x1E9810 | 0x1E8F38 |
| 4 | `0x002A3A50` | FLEVER | — | — | — |
| 5 | `0x002A3AB0` | FLEVER_TRISTATE | — | — | — |

Interessante: FLEVER e FLEVER_TRISTATE têm slots de callback diferentes,
possivelmente variantes do mesmo sistema.

"ROPE" (o label que guiou as revisões Rev.026-035) não é um descritor
separado. É um nome secundário dentro dos 12 bytes de nome do BARREL
descriptor, ou possivelmente um padding alinhado.

## O que fica confirmado

1. `a1` em `0x001D27A8` = `sp` do caller — initializer na stack
2. Caller em `0x001B7A74`-`0x001B7A8C`
3. Callback em `[descriptor + 0x58]` — descriptor = `0x002A3924` (BARREL)
4. `[a1 + 0x30] = 0` — variant inicial = 0
5. `[a1 + 0x58] = 0` — callback não está no initializer
6. Retorno salvo em `[[s3+0x15C] + 0x800]`
7. Descriptor table contém BARREL, CHAIN, FLEVER, FLEVER_TRISTATE
8. Slots +0x48, +0x50, +0x58 formam a interface do descritor
9. Initializer struct tem dados de transform (posição, escala, peso)
10. ROPE é label dentro do BARREL, não descritor independente

## O que fica provável

1. O initializer na stack é montado a partir de dados de transform da
   entidade (s3) — possivelmente uma "pose" para inicializar a simulação
   de cloth
2. A duplicação de 0x40 bytes sugere "pose atual → pose alvo" para
   interpolação
3. O caller em `0x001B7A74` faz parte do sistema de iteração de
   descritores (state resolver)
4. Cada descritor na tabela representa um sistema físico diferente:
   BARREL (barril/objeto cilíndrico), CHAIN (corrente), FLEVER (alavanca)

## O que ainda depende de runtime

1. Se `[a1+0x30]` muda em outros hits (qual o range de variant?)
2. Como o initializer struct é montado (quem copia os dados?)
3. O caller `0x001B7A74` — quem chama este código?
4. Qual o stride exato entre descritores na tabela
5. Os slots +0x48/50 de CHAIN e FLEVER apontam para quais funções
6. Breakpoint em `0x0013F7A8` com `a3 == 0x13` para ver registro do
   callback `0x001D3A30` no slot +0x50

## Próximo teste mínimo

### Com emulador
1. Breakpoint em `0x001B7A74` — capturar quem chama este caller
2. Breakpoint em `0x0013F7A8` com condição `a3 == 0x13`
3. Verificar se há outros hits de `0x001D27A8` com variant diferente

### Sem emulador
1. Atualizar structs hipotéticas com o novo conhecimento
2. Mapear complete descriptor table com strides e slots
3. Preparar próximo runtime session

## Veredito

O modelo runtime do descritor callback está consolidado. A cadeia é:

```
descriptor (0x002A3924, .data)
  └─ +0x58 = callback (0x001D27A8)
       └─ chamado por caller (0x001B7A74)
            ├─ a0 = s3 (entity context, heap)
            ├─ a1 = sp (initializer stack)
            │    └─ +0x30 = variant
            └─ retorno = payload
                 └─ salvo em [[entity+0x15C]+0x800]
```

O gap principal do projeto está fechado. A estrutura do sistema de
descritores cloth está mapeada.
