# Descriptor Table Analysis — BARREL, ROPE, CHAIN, FLEVER

## Date

2026-05-15

## Resumo executivo

Análise da tabela de descritores na `.data` capturada via PCSX2 debugger
durante a sessão runtime Rev.046. A tabela contém registros de sistemas
de física/animação com slots de callback.

## Dump capturado

```
0x002A3920: 00 00 00 00 42 41 52 52 45 4C 00 00 00 00 00 00  |....BARREL......|
0x002A3930: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  |................|
0x002A3940: 00 00 00 00 F3 00 00 00 00 00 80 BF 00 00 80 3F  |.......?.......?|
0x002A3950: 0A D7 23 3C 01 00 00 00 00 00 00 00 00 00 00 00  |..#<............|
0x002A3960: 00 00 00 00 00 00 00 00 01 00 00 00 28 3B 1D 00  |............(;..|
0x002A3970: 00 00 00 00 30 3A 1D 00 00 00 00 00 A8 27 1D 00  |....0:.......'..|
0x002A3980: 00 00 00 00 00 00 00 00 52 4F 50 45 00 00 00 00  |........ROPE....|
0x002A3990: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  |................|

0x002A39A0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 BF  |................|
0x002A39B0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  |................|
0x002A39C0: 00 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00  |................|
0x002A39D0: 30 96 1E 00 00 00 00 00 10 98 1E 00 00 00 00 00  |0...............|
0x002A39E0: 38 8F 1E 00 00 00 00 00 00 00 00 00 43 48 41 49  |8...........CHAI|
0x002A39F0: 4E 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  |N...............|

0x002A3A50: 46 4C 45 56 45 52 00 00 00 00 00 00 00 00 00 00  |FLEVER..........|
0x002A3AB0: 46 4C 45 56 45 52 5F 54 52 49 53 54 41 54 45 00  |FLEVER_TRISTATE.|
```

## Estrutura de cada descritor

Cada entry parece ter o seguinte layout (total ~0x60 bytes):

| Offset | Tamanho | Campo | Exemplo (BARREL) |
|---|---|---|---|
| +0x00 | 4 | flags/ID | `0x00000000` |
| +0x04 | 12 | **nome (12 bytes)** | `"BARREL\0\0\0\0\0\0\0"` |
| +0x10 | 4 | ? | `0x00000000` |
| +0x14 | 4 | ? | `0x00000000` |
| +0x18 | 4 | ? | `0x00000000` |
| +0x1C | 4 | ? | `0x000000F3` |
| +0x20 | 4 | float? | `-1.0f` (0xBF800000) |
| +0x24 | 4 | float? | `1.0f` (0x3F800000) |
| +0x28 | 4 | ? | `0x3C23D70A` (~0.01f) |
| +0x2C | 4 | ? | `0x00000001` |
| +0x30-37 | 8 | zero | `0x0000000000000000` |
| +0x38 | 4 | ? | `0x00000000` |
| +0x3C | 4 | ? | `0x00000000` |
| +0x40 | 4 | count? | `0x00000001` |
| +0x44 | 4 | **slot +0x48** | `0x001D3B28` (cloth_aux) |
| +0x4C | 4 | **slot +0x50** | `0x001D3A30` (cloth_update_cb) |
| +0x54 | 4 | **slot +0x58** | `0x001D27A8` (cloth_payload_init) |
| +0x5C | 4 | ? | `0x00000000` |
| +0x60 | 12 | **nome (repetido?)** | `"ROPE\0\0\0\0\0\0\0\0"` |

### Slots de callback

Os 3 slots formam uma **vtable** do descritor:

| Slot | Offset | BARREL value | Função | Propósito |
|---|---|---|---|---|
| +0x48 | +0x44 | `0x001D3B28` | cloth_aux | Auxiliar |
| +0x50 | +0x4C | `0x001D3A30` | cloth_update_cb | Update por frame ** |
| +0x58 | +0x54 | `0x001D27A8` | cloth_payload_init | Inicialização (Runtime Rev.046) |

** O callback `0x001D3A30` é o "ROPE callback" de todas as revisões anteriores.
Seu nome no descritor é uma pista falsa — o nome "ROPE" está no BARREL descriptor,
não é um descritor separado.

### Lista de descritores encontrados

| Descritor | Endereço | Nome |
|---|---|---|
| 1 | `0x002A3924` | **BARREL** (s0 da captura runtime) |
| 2 | `0x002A3988` | ROPE (dentro do BARREL) |
| 3 | `0x002A39E8` | **CHAIN** |
| 4 | `0x002A3A50` | **FLEVER** |
| 5 | `0x002A3AB0` | **FLEVER_TRISTATE** |

### Observações

1. **"ROPE" não é um descritor separado** — é um nome secundário dentro do
   BARREL descriptor (possivelmente um subtipo ou label extra)
2. **Os 3 slots (+0x48/50/58) formam uma interface** — todo descritor com
   estes slots pode ter init, update, aux
3. **O stride entre descritores** parece ser 0x60 bytes (nome de 12 bytes
   + campos + slots + padding)
4. **FLEVER e FLEVER_TRISTATE** são nomes em holandês (flever = arrow/bolt,
   possivelmente nomes de sistemas internos Team Ico)

## Slot +0x58 resolve a cadeia runtime

A captura Rev.046 mostrou que `s0 = 0x002A3924` (BARREL descriptor)
e `[s0 + 0x58] = 0x001D27A8`. A tabela acima confirma:

```
BARREL[+0x58] = 0x001D27A8 → cloth_payload_init ✓
BARREL[+0x50] = 0x001D3A30 → cloth_update_cb ✓
BARREL[+0x48] = 0x001D3B28 → cloth_aux ✓
```

A cadeia completa fecha.
