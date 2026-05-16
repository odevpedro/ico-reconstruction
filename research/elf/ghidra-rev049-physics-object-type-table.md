# Rev.049 — Physics Object Type Table at 0x001A48A0

## Data

2026-05-16

## Objetivo

Documentar a tabela de tipos de objeto físico (stride 0x64, 31+ entries)
descoberta em `0x001A48A0`, que contém a entrada "ROPE" com os handlers
`0x001D3B28`, `0x001D3A30` e `0x001D27A8`.

Esta tabela é distinta da tabela de variante cloth em `0x004D4188`
(documentada em Rev.041, stride 0x14, 8 entries).

## Escopo

Incluído:

- delimitação e estrutura da tabela `0x001A48A0`;
- lista completa de tipos com seus 3 handlers;
- relação com o callback ROPE `0x001D3A30` e `0x001D27A8` (cloth_payload_init);
- implicações para o ROPE registration gap.

Excluído:

- runtime/emulador;
- análise da função que itera a tabela;
- `DATA.DF`;
- `.gcm`;
- análise semântica detalhada de cada tipo;
- matching PAL/USA.

## Fontes usadas

| Fonte | Uso |
|---|---|
| `.local/extracted/SCUS_971.13.elf` | bytes e instruções |
| `research/elf/ghidra-rev041-cloth-variant-table-004d4188.md` | distinção entre as duas tabelas |
| `research/runtime/pcsx2-probe-session2-2026-05-16-cablecar.jsonl` | confirmação de que `0x001D3A30` não passa por `0x0013F7A8` |

## Descoberta

A busca por `0x001D3A30` como dado (word de 32 bits little-endian) no ELF
inteiro encontrou **exatamente 1 ocorrência**, no offset `0x001A4974`.

A região ao redor revelou uma tabela de tipos de objeto físico com a
seguinte estrutura:

### Estrutura do entry

Cada entry tem stride **0x64** (100 bytes):

```
+0x00: u32  count          (geralmente 0x00000001)
+0x04: u32  func_handler_a (primeiro handler)
+0x08: u32  null           (0x00000000 separador)
+0x0C: u32  func_handler_b (segundo handler)
+0x10: u32  null           (0x00000000 separador)
+0x14: u32  func_handler_c (terceiro handler)
+0x18: u32  null           (0x00000000)
+0x1C: u32  null           (0x00000000)
+0x20: char name[8]        (nome ASCII do tipo, ex: "ROPE\0\0\0\0")
```

Para o entry **ROPE** em `0x001A4968`:

| Offset | Valor | Significado |
|---|---|---|
| +0x00 | `0x00000001` | count |
| +0x04 | `0x001D3B28` | handler_a (pós-dispatch/cleanup?) |
| +0x08 | `0x00000000` | separador |
| +0x0C | `0x001D3A30` | handler_b **(ROPE callback, chama dispatcher 0x001D37C8)** |
| +0x10 | `0x00000000` | separador |
| +0x14 | `0x001D27A8` | handler_c **(cloth_payload_init, confirmado em runtime)** |
| +0x18 | `0x00000000` | padding |
| +0x1C | `0x00000000` | padding |
| +0x20 | `"ROPE\0\0\0\0"` | nome do tipo |

### Tabela completa

A primeira entrada da tabela está em `0x001A48A0` (ROTOBJEC).
Antes disso há WOODBOX01 (estrutura ligeiramente diferente, sem os 3 handlers).

Foram identificados **31 entries** no total:

| Offset | Nome | handler_a | handler_b | handler_c |
|---|---|---|---|---|
| `0x001A48A0` | ROTOBJEC | `0x001C05D0` | `0x001C0538` | `0x001C00C0` |
| `0x001A4904` | BARREL | `0x001EA030` | `0x001E9950` | `0x001E9F08` |
| `0x001A4968` | **ROPE** | **`0x001D3B28`** | **`0x001D3A30`** | **`0x001D27A8`** |
| `0x001A49CC` | CHAIN | `0x001E9630` | `0x001E9810` | `0x001E8F38` |
| `0x001A4A30` | FLEVER | `0x0018F640` | `0x0018ECC8` | `0x0018E5B0` |
| `0x001A4AF8` | WLEVER | `0x001BC438` | `0x001BC130` | `0x001C09C8` |
| `0x001A4BC0` | NONE | `0x001BC7F0` | `0x001BC530` | `0x001C0C40` |
| `0x001A4C88` | CAMERADU | `0x0010EC60` | (null) | (null) |
| `0x001A4D50` | DUMMY? | (null) | (null) | (null) |
| `0x001A4DB4` | SEFFECT | (null) | (null) | (null) |
| `0x001A4E18` | BIRD | `0x001EF988` | `0x001EF980` | `0x001EF8E8` |
| `0x001A4E7C` | GENERATO | `0x00197080` | `0x00197078` | `0x00197240` |
| `0x001A4EE0` | CANDLE | `0x00193600` | `0x001930B0` | `0x00193730` |
| `0x001A4F44` | MOBJ | `0x001C3130` | `0x001C3178` | `0x001C2FF0` |
| `0x001A500C` | CHANDELI | `0x0010ECB0` | `0x001F44C8` | `0x001F43D0` |
| `0x001A5070` | WORM | `0x001C3470` | `0x001C33D8` | `0x001C34B8` |
| `0x001A50D4` | POOL | `0x001F69B0` | `0x001F66F0` | `0x001F62E8` |
| `0x001A5138` | DARKVOLU | `0x0010C5C0` | `0x0010D070` | `0x0010B2D0` |
| `0x001A519C` | MCOLTEST | `0x001CBD78` | `0x001CBD70` | `0x001CBD68` |
| `0x001A5200` | ROPEFIX | `0x001E6960` | `0x001E6788` | `0x001E6968` |
| `0x001A5264` | CAGE | `0x001E98C8` | `0x001E9888` | `0x001E9910` |
| `0x001A52C8` | DYNAMICM | `0x001C2DF8` | `0x001C28D0` | `0x001C2338` |
| `0x001A5390` | QUEEN | `0x001D01E8` | `0x001D00F8` | `0x001CFB58` |
| `0x001A53F4` | QUEENDEM | `0x0019A9A0` | `0x0019A8F0` | `0x0019A7E8` |
| `0x001A5520` | CAGEFIX | `0x001E0860` | `0x001E08B8` | (null) |
| `0x001A5584` | CLOTHTES | `0x001C2FA0` | `0x001C2F20` | `0x001C2FE8` |

### Observações sobre os handlers

- Os handlers de **ROPE** estão todos no range `0x001Dxxxx` (cloth domain):
  `0x001D3B28` está imediatamente após o dispatcher `0x001D37C8` e seus 5
  state blocks (`0x001D3818` a `0x001D3A10`). Função retorno/cleanup após dispatch.
  `0x001D3A30` é o ROPE callback: chama o dispatcher. Confirmado em runtime (Rev.025).
  `0x001D27A8` é cloth_payload_init. Confirmado em runtime (dados espaciais, Rev.045-046).

- **ROPEFIX** é uma variante com handlers em `0x001E6xxx` (overlay),
  possivelmente rope fixo/não-físico.

- **BARREL** tem handlers em `0x001EAxxx` (overlay), sem relação com cloth.

- **FLEVER** e **WLEVER** (levers) têm handlers nos ranges `0x0018xxxx`
  e `0x001Bxxxx`-`0x001Cxxxx`.

- **NONE** é um tipo nulo, sem handlers ativos (`0x001BCxxx` range de fallback).

- **CAMERADU** tem apenas um handler (`0x0010EC60`) — provavelmente dummy/camera.

## Validação da não-registração via 0x0013F7A8

Com 483 capturas de `0x0013F7A8` em sessão de gameplay do início ao bondinho,
nenhum evento registrou `0x001D3A30` como callback. A variante "ROPE" existe
estaticamente na tabela `0x001A4968`, com os 3 handlers inline.

**Conclusão: `0x001D3A30` não é registrado dinamicamente via `0x0013F7A8`.**
Ele é chamado indiretamente a partir da tabela de tipos de objeto físico.

## Implicações para o ROPE gap

- O caminho via `0x0013F7A8` (investigado em Rev.033-038 e Rev.046) está
  descartado para o ROPE callback.
- O ROPE gap (entre `0x001D3A30` confirmado em runtime e seu ponto de chamada)
  se reduz a: **encontrar a função que lê a tabela `0x001A48A0` e invoca o
  handler_b do entry ROPE**.
- A função que itera a tabela **não foi encontrada estaticamente**:
  - Nenhuma ocorrência de LUI+ADDIU para `0x001A48A0` ou `0x001A4968` em
    código.
  - Nenhuma ocorrência de LUI+ADDIU para construir o endereço `0x001D3A30`
    em código (apenas como dado).
  - A tabela é provavelmente acessada via GP-relative, indiretamente, ou
    a função iteradora está em overlay.

## Próximo teste

1. Se runtime for retomado: adicionar probe em `0x001D3A30` (entry point)
   para capturar quem chama (ra, a0-a3, sp). Isso identificará a função
   iteradora da tabela de tipos.
2. Alternativamente: procurar estaticamente por código que acessa as
   funções do entry ROPE via `lw $reg, offset($base)` + `jalr $reg`.
3. Verificar se `0x001A48A0` é referenciada via GP ($28).

## Veredito conservador

A tabela de tipos de objeto físico em `0x001A48A0` (stride 0x64) existe,
contém 31 tipos, e o tipo "ROPE" carrega os handlers do cluster cloth-domain
incluindo `0x001D3A30` e `0x001D27A8`. O ROPE callback não passa por
`0x0013F7A8`. A função que itera esta tabela e invoca os handlers não foi
identificada estaticamente nesta revisão.

## Arquivos relacionados

- `research/elf/ghidra-rev041-cloth-variant-table-004d4188.md`: tabela de
  variante cloth (estrutura diferente, stride 0x14).
- `research/elf/ghidra-rev037-remaining-callers-and-rope-gap.md`: análise
  dos 5 callsites de `0x0013F7A8`, agora descartados para ROPE.
- `research/runtime/pcsx2-probe-session2-2026-05-16-cablecar.jsonl`: 501
  eventos de runtime da sessão de bondinho.
